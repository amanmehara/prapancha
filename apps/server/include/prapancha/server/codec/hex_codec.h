//
// Created by Aman Mehara on 13/03/26.
//

#ifndef PRAPANCHA_CODEC_HEX_CODEC_H_
#define PRAPANCHA_CODEC_HEX_CODEC_H_

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <prapancha/server/codec/codec.h>
#include <prapancha/server/uuid.h>

namespace mehara::prapancha::codec {

    inline constexpr std::string_view hex_chars = "0123456789abcdef";

    template<typename T>
    struct HexCodec;

    template<>
    struct HexCodec<std::vector<std::uint8_t>> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const std::vector<std::uint8_t> &bytes) {
            std::string result;
            result.reserve(bytes.size() * 2);
            for (const std::uint8_t byte: bytes) {
                result.push_back(hex_chars[byte >> 4]);
                result.push_back(hex_chars[byte & 0x0F]);
            }
            return result;
        }

        static std::optional<std::vector<std::uint8_t>> decode(encoded_view data) {
            if (data.length() % 2 != 0) {
                return std::nullopt;
            }
            std::vector<std::uint8_t> bytes;
            bytes.reserve(data.length() / 2);
            for (std::size_t i = 0; i < data.length(); i += 2) {
                std::uint8_t byte = 0;
                if (auto [ptr, ec] = std::from_chars(data.data() + i, data.data() + i + 2, byte, 16);
                    ec != std::errc{} || ptr != data.data() + i + 2) {
                    return std::nullopt;
                }
                bytes.push_back(byte);
            }
            return bytes;
        }
    };

    template<>
    struct HexCodec<UUID> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const UUID &uuid) {
            const auto &bytes = uuid.data();
            std::string result;
            result.reserve(UUID::hex_length);
            for (const std::uint8_t byte: bytes) {
                result.push_back(hex_chars[byte >> 4]);
                result.push_back(hex_chars[byte & 0x0F]);
            }
            return result;
        }

        static std::optional<UUID> decode(encoded_view data) {
            if (data.length() != UUID::hex_length) {
                return std::nullopt;
            }
            UUID::Bytes bytes;
            for (std::size_t i = 0; i < UUID::bytes_length; ++i) {
                std::uint8_t byte = 0;
                if (auto [ptr, ec] = std::from_chars(data.data() + (i * 2), data.data() + (i * 2) + 2, byte, 16);
                    ec != std::errc{} || ptr != data.data() + (i * 2) + 2) {
                    return std::nullopt;
                }
                bytes[i] = byte;
            }
            return UUID(bytes);
        }
    };

    static_assert(Codec<HexCodec<std::vector<std::uint8_t>>, std::vector<std::uint8_t>>);
    static_assert(Codec<HexCodec<UUID>, UUID>);

} // namespace mehara::prapancha::codec

#endif // PRAPANCHA_CODEC_HEX_CODEC_H_
