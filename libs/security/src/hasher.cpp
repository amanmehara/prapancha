//
// Created by Aman Mehara on 21/02/26.
//
#include <prapancha/security/hasher.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <string_view>
#include <vector>

#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>
#include <openssl/rand.h>

namespace mehara::prapancha::security {

    namespace {

        constexpr std::uint32_t argon2id_version = 0x13;
        constexpr std::uint32_t argon2id_m = 65536;
        constexpr std::uint32_t argon2id_t = 3;
        constexpr std::uint32_t argon2id_p = 4;
        constexpr std::size_t argon2id_salt_length = 16;
        constexpr std::size_t argon2id_hash_length = 32;

        auto construct_algorithm_params(std::string_view password, const std::uint32_t *version, const std::uint32_t *m,
                                        const std::uint32_t *t, const std::uint32_t *p,
                                        const std::vector<uint8_t> &salt, Argon2id) noexcept
                -> std::array<OSSL_PARAM, 7> {
            return {OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_PASSWORD, const_cast<char *>(password.data()),
                                                      password.size()),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_VERSION, const_cast<uint32_t *>(version)),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, const_cast<uint32_t *>(m)),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, const_cast<uint32_t *>(t)),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, const_cast<uint32_t *>(p)),
                    OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, const_cast<uint8_t *>(salt.data()),
                                                      salt.size()),
                    OSSL_PARAM_construct_end()};
        }

    } // namespace

    std::expected<Argon2idBinding, Error> Hasher::hash(const std::string_view data, const Argon2id algorithm) {
        ERR_clear_error();
        const auto kdf = std::unique_ptr<EVP_KDF, decltype(&EVP_KDF_free)>(
                EVP_KDF_fetch(nullptr, Argon2id::name.data(), nullptr), EVP_KDF_free);
        if (!kdf) {
            return std::unexpected(Error(ERR_get_error()));
        }
        const auto ctx =
                std::unique_ptr<EVP_KDF_CTX, decltype(&EVP_KDF_CTX_free)>(EVP_KDF_CTX_new(kdf.get()), EVP_KDF_CTX_free);
        if (!ctx) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> salt(argon2id_salt_length);
        if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> hash(argon2id_hash_length);
        const auto params = construct_algorithm_params(data, &argon2id_version, &argon2id_m, &argon2id_t, &argon2id_p,
                                                       salt, algorithm);
        if (EVP_KDF_derive(ctx.get(), hash.data(), hash.size(), params.data()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        return Argon2idBinding{argon2id_version, argon2id_m, argon2id_t, argon2id_p, std::move(salt), std::move(hash)};
    }

    std::expected<bool, Error> Hasher::verify(const std::string_view data, const Argon2idBinding &binding,
                                              const Argon2id algorithm) {
        ERR_clear_error();
        const auto kdf = std::unique_ptr<EVP_KDF, decltype(&EVP_KDF_free)>(
                EVP_KDF_fetch(nullptr, Argon2id::name.data(), nullptr), EVP_KDF_free);
        if (!kdf) {
            return std::unexpected(Error(ERR_get_error()));
        }
        const auto ctx =
                std::unique_ptr<EVP_KDF_CTX, decltype(&EVP_KDF_CTX_free)>(EVP_KDF_CTX_new(kdf.get()), EVP_KDF_CTX_free);
        if (!ctx) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> check(binding.hash.size());
        const auto params = construct_algorithm_params(data, &binding.version, &binding.m, &binding.t, &binding.p,
                                                       binding.salt, algorithm);
        if (EVP_KDF_derive(ctx.get(), check.data(), check.size(), params.data()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        const bool match = CRYPTO_memcmp(check.data(), binding.hash.data(), check.size()) == 0;
        OPENSSL_cleanse(check.data(), check.size());
        return match;
    }

    std::expected<Sha256Binding, Error> Hasher::hash(const std::string_view data, Sha256) {
        ERR_clear_error();
        std::vector<uint8_t> hash(Sha256::digest_length);
        if (!EVP_Q_digest(nullptr, Sha256::name.data(), nullptr, data.data(), data.size(), hash.data(), nullptr)) {
            return std::unexpected(Error(ERR_get_error()));
        }
        return Sha256Binding{std::move(hash)};
    }

    std::expected<Sha256Binding, Error> Hasher::hash(std::istream &stream, Sha256) {
        ERR_clear_error();
        constexpr std::size_t stream_buffer_size = 4096;
        const auto ctx = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);
        if (!ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        char buffer[stream_buffer_size];
        while (stream.read(buffer, sizeof(buffer)) || stream.gcount() > 0) {
            if (EVP_DigestUpdate(ctx.get(), buffer, stream.gcount()) <= 0) {
                return std::unexpected(Error(ERR_get_error()));
            }
        }
        std::vector<uint8_t> hash(Sha256::digest_length);
        if (EVP_DigestFinal_ex(ctx.get(), hash.data(), nullptr) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        return Sha256Binding{std::move(hash)};
    }

    std::expected<bool, Error> Hasher::verify(const std::string_view data, const Sha256Binding &binding, Sha256) {
        return hash(data, Sha256{}).and_then([&binding](const Sha256Binding &result) -> std::expected<bool, Error> {
            if (result.hash.size() != binding.hash.size()) {
                return false;
            }
            return CRYPTO_memcmp(result.hash.data(), binding.hash.data(), binding.hash.size()) == 0;
        });
    }

    std::expected<bool, Error> Hasher::verify(std::istream &stream, const Sha256Binding &binding, Sha256) {
        return hash(stream, Sha256{}).and_then([&binding](const Sha256Binding &result) -> std::expected<bool, Error> {
            if (result.hash.size() != binding.hash.size()) {
                return false;
            }
            const bool match = CRYPTO_memcmp(result.hash.data(), binding.hash.data(), binding.hash.size()) == 0;
            OPENSSL_cleanse(const_cast<uint8_t *>(result.hash.data()), result.hash.size());
            return match;
        });
    }

} // namespace mehara::prapancha::security
