//
// Created by Aman Mehara on 04/02/26.
//

#ifndef PRAPANCHA_UUID_H
#define PRAPANCHA_UUID_H

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace mehara::prapancha {

    /**
     * UUID Class
     * Encapsulates a 16-byte Universally Unique Identifier.
     * Supports Version 7 (Unix Epoch Timestamp + Randomness) for time-ordered sorting.
     */
    class UUID {
    public:
        using raw_t = std::array<uint8_t, 16>;

    private:
        raw_t _data;

    public:
        // Constructors
        UUID();
        explicit UUID(const raw_t &data);

        // Factories
        /**
         * Generates a new UUID v7.
         * Prefix is 48-bit timestamp (ms), followed by 74 bits of entropy.
         */
        static UUID generate();

        /**
         * Creates a UUID from a 32-character hex string (case-insensitive).
         */
        static std::optional<UUID> from_hex(std::string_view hex);

        // Conversions
        [[nodiscard]] std::string to_hex() const;
        [[nodiscard]] const raw_t &raw() const { return _data; }

        /**
         * Extracts the millisecond timestamp encoded in a v7 UUID.
         */
        [[nodiscard]] uint64_t timestamp_ms() const;

        // Comparison operators (C++20 spaceship operator for full ordering)
        auto operator<=>(const UUID &other) const = default;
        bool operator==(const UUID &other) const = default;
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_UUID_H
