//
// Created by Aman Mehara on 10/03/26.
//

#include <prapancha/security/signer.h>

#include <cstddef>
#include <memory>
#include <vector>

#include <openssl/err.h>
#include <openssl/evp.h>

#include <prapancha/security/error.h>

namespace mehara::prapancha::security {

    std::expected<Ed25519Binding, Error> Signer::sign(const std::vector<uint8_t> &data,
                                                      const std::vector<uint8_t> &private_key, Ed25519) {
        ERR_clear_error();
        const auto pkey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>(
                EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr, private_key.data(), private_key.size()),
                EVP_PKEY_free);
        if (!pkey) {
            return std::unexpected(Error(ERR_get_error()));
        }
        const auto ctx = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);
        if (!ctx || EVP_DigestSignInit(ctx.get(), nullptr, nullptr, nullptr, pkey.get()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::size_t signature_length = 0;
        if (EVP_DigestSign(ctx.get(), nullptr, &signature_length, data.data(), data.size()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> signature(signature_length);
        if (EVP_DigestSign(ctx.get(), signature.data(), &signature_length, data.data(), data.size()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        return Ed25519Binding{std::move(signature)};
    }

    std::expected<bool, Error> Signer::verify(const std::vector<uint8_t> &data, const Ed25519Binding &binding,
                                              const std::vector<uint8_t> &public_key, Ed25519) {
        ERR_clear_error();
        const auto pkey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>(
                EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, public_key.data(), public_key.size()),
                EVP_PKEY_free);
        if (!pkey) {
            return std::unexpected(Error(ERR_get_error()));
        }
        const auto ctx = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);
        if (!ctx || EVP_DigestVerifyInit(ctx.get(), nullptr, nullptr, nullptr, pkey.get()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        const int result = EVP_DigestVerify(ctx.get(), binding.signature.data(), binding.signature.size(), data.data(),
                                            data.size());
        if (result < 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        return result == 1;
    }

} // namespace mehara::prapancha::security
