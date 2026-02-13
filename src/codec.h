//
// Created by Aman Mehara on 10/02/26.
//

#ifndef PRAPANCHA_CODEC_H
#define PRAPANCHA_CODEC_H

#include <chrono>
#include <concepts>
#include <optional>
#include <string>
#include <vector>

#include <json/json.h>

#include "codec.h"
#include "model.h"
#include "uuid.h"

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

    template<>
    struct JsonCodec<Author> {
        using EncodedType = Json::Value;

        static EncodedType encode(const Author &model) {
            Json::Value j;
            j["id"] = model.id.to_hex();
            j["display_name"] = model.state.display_name;
            j["bio"] = model.state.bio;
            j["version"] = model.version;
            j["created_at"] =
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count();
            return j;
        }

        static std::optional<Author> decode(const EncodedType &j) {
            if (!j.isMember("display_name") || !j.isMember("bio")) {
                return std::nullopt;
            }
            Author::State state{j["display_name"].asString(), j["bio"].asString()};
            if (j.isMember("id") && j.isMember("version")) {
                return Author::rehydrate(UUID::from_hex(j["id"].asString()).value(), j["version"].asUInt64(),
                                         Timestamp{std::chrono::milliseconds{j["created_at"].asUInt64()}},
                                         std::move(state));
            }
            return Author::create(std::move(state));
        }
    };

    template<>
    struct JsonCodec<Post> {
        using EncodedType = Json::Value;

        static EncodedType encode(const Post &model) {
            Json::Value j;
            j["id"] = model.id.to_hex();
            j["version"] = model.version;
            j["created_at"] =
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count();
            j["author_id"] = model.state.author_id.to_hex();
            j["title"] = model.state.title;
            j["content"] = model.state.content;
            return j;
        }

        static std::optional<Post> decode(const EncodedType &j) {
            if (!j.isMember("author_id") || !j.isMember("title") || !j.isMember("content")) {
                return std::nullopt;
            }
            Post::State state{UUID::from_hex(j["author_id"].asString()).value(), j["title"].asString(),
                              j["content"].asString()};
            if (j.isMember("id") && j.isMember("version") && j.isMember("created_at")) {
                return Post::rehydrate(UUID::from_hex(j["id"].asString()).value(), j["version"].asUInt64(),
                                       Timestamp{std::chrono::milliseconds{j["created_at"].asUInt64()}},
                                       std::move(state));
            }
            return Post::create(std::move(state));
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CODEC_H
