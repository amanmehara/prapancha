//
// Created by Aman Mehara on 04/02/26.
//

#ifndef PRAPANCHA_SERVER_UUID_H_
#define PRAPANCHA_SERVER_UUID_H_

#include <array>

namespace mehara::prapancha {

    /// UUID Class
    ///
    /// Encapsulates a 16-byte Universally Unique Identifier.
    /// Supports Version 7 (Unix Epoch Timestamp + Randomness) for time-ordered sorting.
    class UUID {
    public:
        static constexpr std::size_t bytes_length = 16;
        static constexpr std::size_t hex_length = bytes_length * 2;
        using Bytes = std::array<uint8_t, bytes_length>;

    private:
        Bytes data_;

    public:
        UUID();
        explicit UUID(const Bytes &data);

        /// Generates a new UUID v7.
        /// Prefix is 48-bit timestamp (ms), followed by 74 bits of entropy.
        static UUID generate();

        [[nodiscard]] const Bytes &data() const noexcept;

        /// Extracts the millisecond timestamp encoded in a v7 UUID.
        [[nodiscard]] uint64_t timestamp_ms() const;

        auto operator<=>(const UUID &other) const = default;
        bool operator==(const UUID &other) const = default;
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_UUID_H_
