//
// Created by Aman Mehara on 21/02/26.
//

#ifndef PRAPANCHA_SECURITY_HASHER_H
#define PRAPANCHA_SECURITY_HASHER_H

#include <expected>
#include <string>
#include <vector>

namespace mehara::prapancha::security {

    enum class Error { LibraryFailure, EntropyFailure, InvalidInput };

    struct PasswordBinding {
        uint32_t version;
        uint32_t m;
        uint32_t t;
        uint32_t p;
        std::vector<uint8_t> salt;
        std::vector<uint8_t> hash;

        bool operator==(const PasswordBinding &) const = default;
    };

    class Hasher {
    public:
        Hasher() = delete;

        static std::expected<PasswordBinding, Error> generate(std::string_view password);

        static bool verify(std::string_view password, const PasswordBinding &binding);
    };

} // namespace mehara::prapancha::security

#endif // PRAPANCHA_SECURITY_HASHER_H
