//
// Created by Aman Mehara on 10/03/26.
//

#ifndef PRAPANCHA_SECURITY_CRYPTER_H_
#define PRAPANCHA_SECURITY_CRYPTER_H_

#include <expected>
#include <vector>

#include <prapancha/security/error.h>

namespace mehara::prapancha::security {

    struct Aes256GcmBinding {
        std::vector<uint8_t> iv;
        std::vector<uint8_t> tag;
        std::vector<uint8_t> ciphertext;
        bool operator==(const Aes256GcmBinding &) const = default;
    };

    struct Aes256Gcm {
        using Binding = Aes256GcmBinding;
    };

    class Crypter {
    public:
        Crypter() = delete;

        template<typename Algorithm>
        static std::expected<typename Algorithm::Binding, Error> encrypt(const std::vector<uint8_t> &plain,
                                                                         const std::vector<uint8_t> &key) {
            return encrypt(plain, key, Algorithm{});
        }

        template<typename Algorithm>
        static std::expected<std::vector<uint8_t>, Error> decrypt(const typename Algorithm::Binding &binding,
                                                                  const std::vector<uint8_t> &key) {
            return decrypt(binding, key, Algorithm{});
        }

    private:
        static std::expected<Aes256GcmBinding, Error> encrypt(const std::vector<uint8_t> &,
                                                              const std::vector<uint8_t> &, Aes256Gcm);
        static std::expected<std::vector<uint8_t>, Error> decrypt(const Aes256GcmBinding &,
                                                                  const std::vector<uint8_t> &, Aes256Gcm);
    };

} // namespace mehara::prapancha::security

#endif // PRAPANCHA_SECURITY_CRYPTER_H_
