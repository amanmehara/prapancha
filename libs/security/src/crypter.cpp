//
// Created by Aman Mehara on 10/03/26.
//

#include <prapancha/security/crypter.h>

#include <memory>
#include <vector>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <prapancha/security/error.h>

namespace mehara::prapancha::security {

    std::expected<Aes256GcmBinding, Error> Crypter::encrypt(const std::vector<uint8_t> &plain,
                                                            const std::vector<uint8_t> &key, Aes256Gcm) {
        ERR_clear_error();
        const auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>(EVP_CIPHER_CTX_new(),
                                                                                         EVP_CIPHER_CTX_free);
        if (!ctx) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> iv(12);
        if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
            return std::unexpected(Error(ERR_get_error()));
        }
        if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> cipher(plain.size());
        int length = 0;
        if (EVP_EncryptUpdate(ctx.get(), cipher.data(), &length, plain.data(), static_cast<int>(plain.size())) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        int final_length = 0;
        if (EVP_EncryptFinal_ex(ctx.get(), cipher.data() + length, &final_length) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> tag(16);
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_AEAD_GET_TAG, 16, tag.data()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        return Aes256GcmBinding{std::move(iv), std::move(tag), std::move(cipher)};
    }

    std::expected<std::vector<uint8_t>, Error> Crypter::decrypt(const Aes256GcmBinding &binding,
                                                                const std::vector<uint8_t> &key, Aes256Gcm) {
        ERR_clear_error();
        const auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>(EVP_CIPHER_CTX_new(),
                                                                                         EVP_CIPHER_CTX_free);
        if (!ctx) {
            return std::unexpected(Error(ERR_get_error()));
        }
        if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.data(), binding.iv.data()) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        std::vector<uint8_t> plain(binding.ciphertext.size());
        int length = 0;
        if (EVP_DecryptUpdate(ctx.get(), plain.data(), &length, binding.ciphertext.data(),
                              static_cast<int>(binding.ciphertext.size())) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_AEAD_SET_TAG, 16, const_cast<uint8_t *>(binding.tag.data())) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        int final_length = 0;
        if (EVP_DecryptFinal_ex(ctx.get(), plain.data() + length, &final_length) <= 0) {
            return std::unexpected(Error(ERR_get_error()));
        }
        return plain;
    }

} // namespace mehara::prapancha::security
