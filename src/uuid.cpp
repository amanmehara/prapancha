//
// Created by Aman Mehara on 04/02/26.
//

#include "uuid.h"

#include <chrono>
#include <random>

namespace mehara::prapancha {

    // Standard lowercase hex map for UUID canonical representation
    static constexpr char HEX_MAP[] = "0123456789abcdef";

    UUID::UUID() : _data{} {}

    UUID::UUID(const raw_t &data) : _data(data) {}

    UUID UUID::generate() {
        raw_t data;

        // Get current Unix timestamp in milliseconds (48 bits)
        const auto now = std::chrono::system_clock::now();
        const uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        // Fill timestamp (Big-Endian / Network Byte Order)
        data[0] = static_cast<uint8_t>(ms >> 40);
        data[1] = static_cast<uint8_t>(ms >> 32);
        data[2] = static_cast<uint8_t>(ms >> 24);
        data[3] = static_cast<uint8_t>(ms >> 16);
        data[4] = static_cast<uint8_t>(ms >> 8);
        data[5] = static_cast<uint8_t>(ms);

        // Fill remaining 10 bytes with randomness
        // thread_local ensures thread safety with high performance
        thread_local std::random_device rd;
        thread_local std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint16_t> dis(0, 255);

        for (size_t i = 6; i < 16; ++i) {
            data[i] = static_cast<uint8_t>(dis(gen));
        }

        // Set Version to 7 (bits 4-7 of byte 6 set to 0111)
        data[6] = (data[6] & 0x0F) | 0x70;

        // Set Variant to RFC 4122 (bits 6-7 of byte 8 set to 10)
        data[8] = (data[8] & 0x3F) | 0x80;

        return UUID(data);
    }

    std::optional<UUID> UUID::from_hex(const std::string_view hex) {
        if (hex.length() != 32)
            return std::nullopt;

        auto char_to_int = [](const char c) -> int {
            if (c >= '0' && c <= '9') {
                return c - '0';
            }
            if (c >= 'a' && c <= 'f') {
                return c - 'a' + 10;
            }
            if (c >= 'A' && c <= 'F') {
                return c - 'A' + 10;
            }
            return -1;
        };

        raw_t bytes;
        for (size_t i = 0; i < 16; ++i) {
            const int high = char_to_int(hex[i * 2]);
            const int low = char_to_int(hex[i * 2 + 1]);

            if (high == -1 || low == -1)
                return std::nullopt;
            bytes[i] = static_cast<uint8_t>((high << 4) | low);
        }
        return UUID(bytes);
    }

    std::string UUID::to_hex() const {
        std::string out;
        out.reserve(32);
        for (const uint8_t byte: _data) {
            out.push_back(HEX_MAP[(byte >> 4) & 0x0F]);
            out.push_back(HEX_MAP[byte & 0x0F]);
        }
        return out;
    }

    uint64_t UUID::timestamp_ms() const {
        uint64_t ms = 0;
        ms |= static_cast<uint64_t>(_data[0]) << 40;
        ms |= static_cast<uint64_t>(_data[1]) << 32;
        ms |= static_cast<uint64_t>(_data[2]) << 24;
        ms |= static_cast<uint64_t>(_data[3]) << 16;
        ms |= static_cast<uint64_t>(_data[4]) << 8;
        ms |= static_cast<uint64_t>(_data[5]);
        return ms;
    }

} // namespace mehara::prapancha
