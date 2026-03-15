//
// Created by Aman Mehara on 14/03/26.
//

#ifndef PRAPANCHA_SERVER_CODEC_BINARY_JSON_CODEC_H_
#define PRAPANCHA_SERVER_CODEC_BINARY_JSON_CODEC_H_

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include <boost/json.hpp>

#include <prapancha/server/codec/binary_codec.h>

namespace mehara::prapancha::codec {

    template<>
    struct BinaryCodec<boost::json::object> {
        using encoded_type = std::vector<std::uint8_t>;
        using encoded_view = std::span<const std::uint8_t>;

        [[nodiscard]] static encoded_type encode(const boost::json::object &json_object) {
            std::string s = boost::json::serialize(json_object);
            return {s.begin(), s.end()};
        }

        [[nodiscard]] static std::optional<boost::json::object> decode(encoded_view data) {
            if (data.empty()) {
                return std::nullopt;
            }
            try {
                const std::string_view sv{reinterpret_cast<const char *>(data.data()), data.size()};
                boost::json::value json_value = boost::json::parse(sv);
                if (!json_value.is_object()) {
                    return std::nullopt;
                }
                return json_value.as_object();
            } catch (...) {
                return std::nullopt;
            }
        }
    };

} // namespace mehara::prapancha::codec

#endif // PRAPANCHA_SERVER_CODEC_BINARY_JSON_CODEC_H_
