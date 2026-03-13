//
// Created by Aman Mehara on 10/02/26.
//

#ifndef PRAPANCHA_SERVER_CODEC_CODEC_H_
#define PRAPANCHA_SERVER_CODEC_CODEC_H_

#include <charconv>
#include <chrono>
#include <concepts>
#include <optional>
#include <string>
#include <vector>

#include <boost/json.hpp>

#include <prapancha/security/hasher.h>
#include <prapancha/server/model.h>
#include <prapancha/server/uuid.h>

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
    struct HexCodec;

    template<>
    struct HexCodec<std::vector<uint8_t>> {
        using EncodedType = std::string;

        static EncodedType encode(const std::vector<uint8_t> &bytes) {
            static constexpr char hex_chars[] = "0123456789abcdef";
            std::string res;
            res.reserve(bytes.size() * 2);
            for (const uint8_t b: bytes) {
                res.push_back(hex_chars[b >> 4]);
                res.push_back(hex_chars[b & 0x0F]);
            }
            return res;
        }

        static std::optional<std::vector<uint8_t>> decode(const EncodedType &data) {
            if (data.length() % 2 != 0) {
                return std::nullopt;
            }
            std::vector<std::uint8_t> bytes;
            bytes.reserve(data.length() / 2);
            for (std::size_t i = 0; i < data.length(); i += 2) {
                std::uint8_t byte = 0;
                if (auto [_, ec] = std::from_chars(data.data() + i, data.data() + i + 2, byte, 16); ec != std::errc{}) {
                    return std::nullopt;
                }
                bytes.push_back(byte);
            }
            return bytes;
        }
    };

    static_assert(Codec<HexCodec<std::vector<std::uint8_t>>, std::vector<std::uint8_t>>);

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
            boost::json::array arr;
            arr.reserve(collection.size());
            for (const auto &item: collection) {
                arr.push_back(boost::json::parse(JsonCodec<T>::encode(item)));
            }
            return boost::json::serialize(arr);
        }

        static std::optional<std::vector<T>> decode(const EncodedType &data) {
            try {
                auto jv = boost::json::parse(data);
                if (!jv.is_array()) {
                    return std::nullopt;
                }
                std::vector<T> result;
                result.reserve(jv.as_array().size());
                for (const auto &element: jv.as_array()) {
                    auto item = JsonCodec<T>::decode(boost::json::serialize(element));
                    if (item) {
                        result.push_back(std::move(*item));
                    }
                }
                return result;
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<>
    struct JsonCodec<security::Argon2idBinding> {
        using EncodedType = std::string;

        static EncodedType encode(const security::Argon2idBinding &binding) {
            boost::json::object obj;
            obj["v"] = binding.version;
            obj["m"] = binding.m;
            obj["t"] = binding.t;
            obj["p"] = binding.p;
            obj["salt"] = HexCodec<std::vector<std::uint8_t>>::encode(binding.salt);
            obj["hash"] = HexCodec<std::vector<std::uint8_t>>::encode(binding.hash);
            return boost::json::serialize(obj);
        }

        static std::optional<security::Argon2idBinding> decode(const EncodedType &data) {
            try {
                auto obj = boost::json::parse(data).as_object();
                auto salt = HexCodec<std::vector<uint8_t>>::decode(std::string(obj.at("salt").as_string()));
                auto hash = HexCodec<std::vector<uint8_t>>::decode(std::string(obj.at("hash").as_string()));
                if (!salt || !hash) {
                    return std::nullopt;
                }
                return security::Argon2idBinding{static_cast<std::uint32_t>(obj.at("v").as_int64()),
                                                 static_cast<std::uint32_t>(obj.at("m").as_int64()),
                                                 static_cast<std::uint32_t>(obj.at("t").as_int64()),
                                                 static_cast<std::uint32_t>(obj.at("p").as_int64()),
                                                 std::move(*salt),
                                                 std::move(*hash)};
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<>
    struct JsonCodec<security::Sha256Binding> {
        using EncodedType = std::string;

        static EncodedType encode(const security::Sha256Binding &binding) {
            boost::json::object obj;
            obj["hash"] = HexCodec<std::vector<uint8_t>>::encode(binding.hash);
            return boost::json::serialize(obj);
        }

        static std::optional<security::Sha256Binding> decode(const EncodedType &data) {
            try {
                auto obj = boost::json::parse(data).as_object();
                auto hash = HexCodec<std::vector<uint8_t>>::decode(std::string(obj.at("hash").as_string()));
                if (!hash) {
                    return std::nullopt;
                }
                return security::Sha256Binding{std::move(*hash)};
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<typename PasswordBinding>
    struct JsonCodec<UserIdentity<PasswordBinding>> {
        using EncodedType = std::string;

        static EncodedType encode(const UserIdentity<PasswordBinding> &model) {
            boost::json::object j;
            j["id"] = model.id.to_hex();
            j["version"] = model.version;
            j["created_at"] = static_cast<uint64_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count());
            j["username"] = model.state.username;
            j["is_admin"] = model.state.is_admin;
            j["password_binding"] =
                    boost::json::parse(JsonCodec<PasswordBinding>::encode(model.state.password_binding));
            return boost::json::serialize(j);
        }

        static std::optional<UserIdentity<PasswordBinding>> decode(const EncodedType &data) {
            try {
                auto j = boost::json::parse(data).as_object();
                auto pb_raw = boost::json::serialize(j.at("password_binding"));
                auto pb_opt = JsonCodec<PasswordBinding>::decode(pb_raw);
                if (!pb_opt || !j.contains("username")) {
                    return std::nullopt;
                }
                typename UserIdentity<PasswordBinding>::State state{std::string(j.at("username").as_string()),
                                                                    std::move(*pb_opt), j.at("is_admin").as_bool()};
                if (j.contains("id") && j.contains("version") && j.contains("created_at")) {
                    return UserIdentity<PasswordBinding>::rehydrate(
                            UUID::from_hex(std::string(j.at("id").as_string())).value(), j.at("version").as_uint64(),
                            Timestamp{std::chrono::milliseconds{j.at("created_at").as_uint64()}}, std::move(state));
                }
                return UserIdentity<PasswordBinding>::create(std::move(state));
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<>
    struct JsonCodec<Author> {
        using EncodedType = std::string;

        static EncodedType encode(const Author &model) {
            boost::json::object j;
            j["id"] = model.id.to_hex();
            j["display_name"] = model.state.display_name;
            j["bio"] = model.state.bio;
            j["version"] = model.version;
            j["created_at"] = static_cast<uint64_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count());
            return boost::json::serialize(j);
        }

        static std::optional<Author> decode(const EncodedType &data) {
            try {
                auto j = boost::json::parse(data).as_object();
                if (!j.contains("display_name") || !j.contains("bio")) {
                    return std::nullopt;
                }
                Author::State state{std::string(j.at("display_name").as_string()),
                                    std::string(j.at("bio").as_string())};
                if (j.contains("id") && j.contains("version") && j.contains("created_at")) {
                    return Author::rehydrate(
                            UUID::from_hex(std::string(j.at("id").as_string())).value(), j.at("version").as_uint64(),
                            Timestamp{std::chrono::milliseconds{j.at("created_at").as_uint64()}}, std::move(state));
                }
                return std::nullopt;
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<>
    struct JsonCodec<Post> {
        using EncodedType = std::string;

        static EncodedType encode(const Post &model) {
            boost::json::object j;
            j["id"] = model.id.to_hex();
            j["author_id"] = model.state.author_id.to_hex();
            j["title"] = model.state.title;
            j["content"] = model.state.content;
            j["version"] = model.version;
            j["created_at"] = static_cast<uint64_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(model.created_at.time_since_epoch()).count());
            return boost::json::serialize(j);
        }

        static std::optional<Post> decode(const EncodedType &data) {
            try {
                auto j = boost::json::parse(data).as_object();
                if (!j.contains("author_id") || !j.contains("title") || !j.contains("content")) {
                    return std::nullopt;
                }
                Post::State state{UUID::from_hex(std::string(j.at("author_id").as_string())).value(),
                                  std::string(j.at("title").as_string()), std::string(j.at("content").as_string())};
                if (j.contains("id") && j.contains("version") && j.contains("created_at")) {
                    return Post::rehydrate(
                            UUID::from_hex(std::string(j.at("id").as_string())).value(), j.at("version").as_uint64(),
                            Timestamp{std::chrono::milliseconds{j.at("created_at").as_uint64()}}, std::move(state));
                }
                return Post::create(std::move(state));
            } catch (...) {
                return std::nullopt;
            }
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_CODEC_CODEC_H_
