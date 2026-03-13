//
// Created by Aman Mehara on 21/02/26.
//

#ifndef PRAPANCHA_SECURITY_HASHER_H_
#define PRAPANCHA_SECURITY_HASHER_H_

#include <cstdint>
#include <expected>
#include <istream>
#include <string_view>
#include <vector>

#include <prapancha/security/error.h>

namespace mehara::prapancha::security {

    struct Argon2idBinding {
        std::uint32_t version, m, t, p;
        std::vector<uint8_t> salt;
        std::vector<uint8_t> hash;
        bool operator==(const Argon2idBinding &) const = default;
    };

    struct Sha256Binding {
        std::vector<uint8_t> hash;
        bool operator==(const Sha256Binding &) const = default;
    };

    struct Argon2id {
        using Binding = Argon2idBinding;
        static constexpr std::string_view name = "ARGON2ID";
    };

    struct Sha256 {
        using Binding = Sha256Binding;
        static constexpr std::string_view name = "SHA256";
        static constexpr std::size_t digest_length = 32;
    };

    class Hasher final {
    public:
        Hasher() = delete;

        template<typename Algorithm>
        static std::expected<typename Algorithm::Binding, Error> hash(std::string_view data) {
            return hash(data, Algorithm{});
        }

        template<typename Algorithm>
        static std::expected<typename Algorithm::Binding, Error> hash(std::istream &stream) {
            return hash(stream, Algorithm{});
        }

        template<typename Algorithm>
        static std::expected<bool, Error> verify(std::string_view data, const typename Algorithm::Binding &binding) {
            return verify(data, binding, Algorithm{});
        }

    private:
        static std::expected<Argon2idBinding, Error> hash(std::string_view, Argon2id);
        static std::expected<bool, Error> verify(std::string_view, const Argon2idBinding &, Argon2id);
        static std::expected<Sha256Binding, Error> hash(std::string_view, Sha256);
        static std::expected<Sha256Binding, Error> hash(std::istream &, Sha256);
        static std::expected<bool, Error> verify(std::string_view, const Sha256Binding &, Sha256);
        static std::expected<bool, Error> verify(std::istream &, const Sha256Binding &, Sha256);
    };

} // namespace mehara::prapancha::security

#endif // PRAPANCHA_SECURITY_HASHER_H_
