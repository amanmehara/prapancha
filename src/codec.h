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
#include "security/hasher.h"
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
        using EncodedType = std::string;

        static EncodedType encode(const T &model) = delete;
        static std::optional<T> decode(const EncodedType &data) = delete;
    };

    template<typename T>
    struct JsonCodec<std::vector<T>> {
        using EncodedType = std::string;

        static EncodedType encode(const std::vector<T> &collection) {
            Json::Value array_json(Json::arrayValue);
            Json::CharReaderBuilder readerBuilder;
            std::string errors;
            for (const auto &item: collection) {
                std::string raw = JsonCodec<T>::encode(item);
                Json::Value element;
                if (auto const reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
                    reader->parse(raw.data(), raw.data() + raw.size(), &element, &errors)) {
                    array_json.append(std::move(element));
                }
            }
            Json::StreamWriterBuilder writerBuilder;
            writerBuilder["indentation"] = "";
            return Json::writeString(writerBuilder, array_json);
        }

        static std::optional<std::vector<T>> decode(const EncodedType &data) {
            Json::Value root;
            const Json::CharReaderBuilder readerBuilder;
            std::string errors;
            if (auto const reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
                !reader->parse(data.data(), data.data() + data.size(), &root, &errors) || !root.isArray()) {
                return std::nullopt;
            }
            std::vector<T> result;
            result.reserve(root.size());
            Json::StreamWriterBuilder writerBuilder;
            writerBuilder["indentation"] = "";
            for (const auto &element: root) {
                auto item = JsonCodec<T>::decode(Json::writeString(writerBuilder, element));
                if (item) {
                    result.push_back(std::move(*item));
                }
            }
            return result;
        }
    };

    template<>
    struct JsonCodec<security::PasswordBinding> {
        using EncodedType = std::string;

        static EncodedType encode(const security::PasswordBinding &pb) {
            Json::Value j;
            j["v"] = pb.version;
            j["m"] = pb.m;
            j["t"] = pb.t;
            j["p"] = pb.p;
            auto to_hex = [](const std::vector<uint8_t> &bytes) {
                std::string res;
                static constexpr char hex_chars[] = "0123456789abcdef";
                for (const uint8_t b: bytes) {
                    res.push_back(hex_chars[b >> 4]);
                    res.push_back(hex_chars[b & 0x0F]);
                }
                return res;
            };
            j["salt"] = to_hex(pb.salt);
            j["hash"] = to_hex(pb.hash);
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            return Json::writeString(builder, j);
        }

        static std::optional<security::PasswordBinding> decode(const EncodedType &data) {
            Json::Value j;
            Json::CharReaderBuilder readerBuilder;
            if (auto const reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
                !reader->parse(data.data(), data.data() + data.size(), &j, nullptr)) {
                return std::nullopt;
            }
            auto from_hex = [](const std::string &hex) {
                std::vector<uint8_t> bytes;
                for (size_t i = 0; i < hex.length(); i += 2) {
                    bytes.push_back(static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
                }
                return bytes;
            };
            return security::PasswordBinding{j["v"].asUInt(),
                                             j["m"].asUInt(),
                                             j["t"].asUInt(),
                                             j["p"].asUInt(),
                                             from_hex(j["salt"].asString()),
                                             from_hex(j["hash"].asString())};
        }
    };

    template<>
    struct JsonCodec<UserIdentity> {
        using EncodedType = std::string;

        static EncodedType encode(const UserIdentity &model) {
            Json::Value j;
            j["id"] = model.id.to_hex();
            j["version"] = model.version;
            j["created_at"] = static_cast<Json::UInt64>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count());
            j["username"] = model.state.username;
            j["is_admin"] = model.state.is_admin;
            std::string pb_raw = JsonCodec<security::PasswordBinding>::encode(model.state.password_binding);
            Json::Value pb_json;
            Json::CharReaderBuilder readerBuilder;
            if (auto const reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
                reader->parse(pb_raw.data(), pb_raw.data() + pb_raw.size(), &pb_json, nullptr)) {
                j["password_binding"] = pb_json;
            }
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            return Json::writeString(builder, j);
        }

        static std::optional<UserIdentity> decode(const EncodedType &data) {
            Json::Value j;
            Json::CharReaderBuilder readerBuilder;
            if (auto const reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
                !reader->parse(data.data(), data.data() + data.size(), &j, nullptr)) {
                return std::nullopt;
            }
            Json::StreamWriterBuilder writerBuilder;
            writerBuilder["indentation"] = "";
            std::string pb_raw = Json::writeString(writerBuilder, j["password_binding"]);
            auto pb_opt = JsonCodec<security::PasswordBinding>::decode(pb_raw);
            if (!pb_opt || !j.isMember("username")) {
                return std::nullopt;
            }
            UserIdentity::State state{j["username"].asString(), std::move(*pb_opt), j["is_admin"].asBool()};
            if (j.isMember("id") && j.isMember("version") && j.isMember("created_at")) {
                return UserIdentity::rehydrate(UUID::from_hex(j["id"].asString()).value(), j["version"].asUInt64(),
                                               Timestamp{std::chrono::milliseconds{j["created_at"].asUInt64()}},
                                               std::move(state));
            }
            return UserIdentity::create(std::move(state));
        }
    };

    template<>
    struct JsonCodec<Author> {
        using EncodedType = std::string;

        static EncodedType encode(const Author &model) {
            Json::Value j;
            j["id"] = model.id.to_hex();
            j["display_name"] = model.state.display_name;
            j["bio"] = model.state.bio;
            j["version"] = model.version;
            j["created_at"] = static_cast<Json::UInt64>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count());
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            return Json::writeString(builder, j);
        }

        static std::optional<Author> decode(const EncodedType &data) {
            Json::Value j;
            Json::CharReaderBuilder readerBuilder;
            if (auto const reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
                !reader->parse(data.data(), data.data() + data.size(), &j, nullptr)) {
                return std::nullopt;
            }
            if (!j.isMember("display_name") || !j.isMember("bio")) {
                return std::nullopt;
            }
            Author::State state{j["display_name"].asString(), j["bio"].asString()};
            if (j.isMember("id") && j.isMember("version") && j.isMember("created_at")) {
                return Author::rehydrate(UUID::from_hex(j["id"].asString()).value(), j["version"].asUInt64(),
                                         Timestamp{std::chrono::milliseconds{j["created_at"].asUInt64()}},
                                         std::move(state));
            }
            return std::nullopt;
        }
    };

    template<>
    struct JsonCodec<Post> {
        using EncodedType = std::string;

        static EncodedType encode(const Post &model) {
            Json::Value j;
            j["id"] = model.id.to_hex();
            j["author_id"] = model.state.author_id.to_hex();
            j["title"] = model.state.title;
            j["content"] = model.state.content;
            j["version"] = model.version;
            j["created_at"] = static_cast<Json::UInt64>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count());
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            return Json::writeString(builder, j);
        }

        static std::optional<Post> decode(const EncodedType &data) {
            Json::Value j;
            const Json::CharReaderBuilder readerBuilder;
            if (auto const reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
                !reader->parse(data.data(), data.data() + data.size(), &j, nullptr)) {
                return std::nullopt;
            }
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
