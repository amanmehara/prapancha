//
// Created by Aman Mehara on 13/03/26.
//

#ifndef PRAPANCHA_CODEC_JSON_CODEC_H_
#define PRAPANCHA_CODEC_JSON_CODEC_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <boost/json.hpp>

#include <prapancha/server/codec/codec.h>

namespace mehara::prapancha::codec {

    template<typename T>
    struct JsonCodec {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const T &model) = delete;
        static std::optional<T> decode(encoded_view data) = delete;
    };

    template<typename T>
    struct JsonCodec<std::vector<T>> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const std::vector<T> &collection) {
            boost::json::array json_array;
            json_array.reserve(collection.size());
            for (const auto &item: collection) {
                json_array.push_back(boost::json::parse(JsonCodec<T>::encode(item)));
            }
            return boost::json::serialize(json_array);
        }

        static std::optional<std::vector<T>> decode(encoded_view data) {
            try {
                auto json_value = boost::json::parse(data);
                if (!json_value.is_array()) {
                    return std::nullopt;
                }
                const auto &json_array = json_value.as_array();
                std::vector<T> result;
                result.reserve(json_array.size());
                for (const auto &element: json_array) {
                    auto item = JsonCodec<T>::decode(boost::json::serialize(element));
                    if (!item) {
                        return std::nullopt;
                    }
                    result.push_back(std::move(*item));
                }
                return result;
            } catch (...) {
                return std::nullopt;
            }
        }
    };

} // namespace mehara::prapancha::codec

#endif // PRAPANCHA_CODEC_JSON_CODEC_H_
