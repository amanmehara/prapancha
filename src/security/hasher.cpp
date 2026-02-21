//
// Created by Aman Mehara on 21/02/26.
//
#include "hasher.h"

#include <memory>
#include <vector>

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>
#include <openssl/rand.h>

namespace mehara::prapancha::security {
    namespace {
        using KDFCtxPtr = std::unique_ptr<EVP_KDF_CTX, decltype(&EVP_KDF_CTX_free)>;

        constexpr std::string_view algorithm = "ARGON2ID";

        auto construct_algorithm_params(std::string_view password, const uint32_t *ver, const uint32_t *m,
                                        const uint32_t *t, const uint32_t *p, const std::vector<uint8_t> &salt) noexcept
                -> std::array<OSSL_PARAM, 7> {
            return {OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_PASSWORD, const_cast<char *>(password.data()),
                                                      password.size()),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_VERSION, const_cast<uint32_t *>(ver)),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, const_cast<uint32_t *>(m)),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, const_cast<uint32_t *>(t)),
                    OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, const_cast<uint32_t *>(p)),
                    OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, const_cast<uint8_t *>(salt.data()),
                                                      salt.size()),
                    OSSL_PARAM_construct_end()};
        }
    } // namespace

    std::expected<PasswordBinding, Error> hasher::generate(std::string_view password) {
        EVP_KDF *kdf = EVP_KDF_fetch(nullptr, algorithm.data(), nullptr);
        if (!kdf) {
            return std::unexpected(Error::LibraryFailure);
        }
        const KDFCtxPtr ctx(EVP_KDF_CTX_new(kdf), EVP_KDF_CTX_free);
        EVP_KDF_free(kdf);
        if (!ctx) {
            return std::unexpected(Error::LibraryFailure);
        }
        std::vector<uint8_t> salt(16);
        if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
            return std::unexpected(Error::EntropyFailure);
        }
        uint32_t version = 0x13, m = 65536, t = 3, p = 4;
        std::vector<uint8_t> hash(32);
        const auto params = construct_algorithm_params(password, &version, &m, &t, &p, salt);
        if (EVP_KDF_derive(ctx.get(), hash.data(), hash.size(), params.data()) <= 0) {
            return std::unexpected(Error::LibraryFailure);
        }
        return PasswordBinding{version, m, t, p, std::move(salt), std::move(hash)};
    }

    bool hasher::verify(std::string_view password, const PasswordBinding &binding) {
        EVP_KDF *kdf = EVP_KDF_fetch(nullptr, algorithm.data(), nullptr);
        if (!kdf) {
            return false;
        }
        const KDFCtxPtr ctx(EVP_KDF_CTX_new(kdf), EVP_KDF_CTX_free);
        EVP_KDF_free(kdf);
        if (!ctx) {
            return false;
        }
        std::vector<uint8_t> check_hash(binding.hash.size());
        const auto params = construct_algorithm_params(password, &binding.version, &binding.m, &binding.t, &binding.p,
                                                       binding.salt);
        if (EVP_KDF_derive(ctx.get(), check_hash.data(), check_hash.size(), params.data()) <= 0) {
            return false;
        }
        const bool is_match = (CRYPTO_memcmp(binding.hash.data(), check_hash.data(), binding.hash.size()) == 0);
        OPENSSL_cleanse(check_hash.data(), check_hash.size());
        return is_match;
    }

} // namespace mehara::prapancha::security
