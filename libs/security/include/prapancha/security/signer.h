//
// Created by Aman Mehara on 10/03/26.
//

#ifndef PRAPANCHA_SECURITY_SIGNER_H_
#define PRAPANCHA_SECURITY_SIGNER_H_

#include <expected>
#include <prapancha/security/error.h>
#include <vector>

namespace mehara::prapancha::security {

    struct Ed25519Binding {
        std::vector<uint8_t> signature;
        bool operator==(const Ed25519Binding &) const = default;
    };
    struct Ed25519 {
        using Binding = Ed25519Binding;
    };

    class Signer {
    public:
        Signer() = delete;

        template<typename Algorithm>
        static std::expected<typename Algorithm::Binding, Error> sign(const std::vector<uint8_t> &data,
                                                                      const std::vector<uint8_t> &private_key) {
            return sign(data, private_key, Algorithm{});
        }

        template<typename Algorithm>
        static std::expected<bool, Error> verify(const std::vector<uint8_t> &data,
                                                 const typename Algorithm::Binding &binding,
                                                 const std::vector<uint8_t> &public_key) {
            return verify(data, binding, public_key, Algorithm{});
        }

    private:
        static std::expected<Ed25519Binding, Error> sign(const std::vector<uint8_t> &, const std::vector<uint8_t> &,
                                                         Ed25519);
        static std::expected<bool, Error> verify(const std::vector<uint8_t> &, const Ed25519Binding &,
                                                 const std::vector<uint8_t> &, Ed25519);
    };

} // namespace mehara::prapancha::security

#endif // PRAPANCHA_SECURITY_SIGNER_H_
