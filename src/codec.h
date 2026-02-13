//
// Created by Aman Mehara on 10/02/26.
//

#ifndef PRAPANCHA_CODEC_H
#define PRAPANCHA_CODEC_H

#include <concepts>
#include <optional>

#include <json/json.h>

#include "codec.h"

namespace mehara::prapancha {

    template<typename C, typename T>
    concept Codec = requires(const T &value, const typename C::EncodedType &data) {
        typename C::EncodedType;
        { C::encode(value) } -> std::same_as<typename C::EncodedType>;
        { C::decode(data) } -> std::same_as<std::optional<T>>;
    };

    template<typename T>
    struct BinaryCodec {};

    template<typename T>
    struct JsonCodec {
        using EncodedType = Json::Value;

        static EncodedType encode(const T &model) { return model.to_json(); }

        static std::optional<T> decode(const EncodedType &data) { return T::from_json(data); }
    };

    template<typename T>
    struct JsonCodec<std::vector<T>> {
        using EncodedType = Json::Value;

        static EncodedType encode(const std::vector<T> &collection) {
            EncodedType array_json(Json::arrayValue);
            for (const auto &item: collection) {
                array_json.append(JsonCodec<T>::encode(item));
            }
            return array_json;
        }

        static std::optional<std::vector<T>> decode(const EncodedType &data) {
            if (!data.isArray()) {
                return std::nullopt;
            }
            std::vector<T> result;
            result.reserve(data.size());
            for (const auto &element: data) {
                auto item = JsonCodec<T>::decode(element);
                if (item) {
                    result.push_back(std::move(*item));
                }
            }
            return result;
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CODEC_H
