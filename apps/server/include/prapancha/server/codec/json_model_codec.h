//
// Created by Aman Mehara on 14/03/26.
//

#ifndef PRAPANCHA_SERVER_CODEC_JSON_MODEL_CODEC_H_
#define PRAPANCHA_SERVER_CODEC_JSON_MODEL_CODEC_H_

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <boost/json.hpp>

#include <prapancha/server/codec/codec.h>
#include <prapancha/server/codec/hex_codec.h>
#include <prapancha/server/codec/json_codec.h>
#include <prapancha/server/model.h>

namespace mehara::prapancha::codec {

    template<typename M>
        requires std::derived_from<M, BaseModel>
    void encode_model_metadata(boost::json::object &json_object, const M &model) {
        const auto &metadata = model.metadata();
        json_object["id"] = HexCodec<UUID>::encode(metadata.id);
        json_object["version"] = metadata.version;
        json_object["created_at"] = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(metadata.created_at.time_since_epoch()).count());
    }

    inline std::optional<BaseModel::Metadata> decode_model_metadata(const boost::json::object &json_object) {
        auto const *id_ptr = json_object.if_contains("id");
        auto const *version_ptr = json_object.if_contains("version");
        auto const *created_at_ptr = json_object.if_contains("created_at");
        if (!id_ptr || !version_ptr || !created_at_ptr) {
            return std::nullopt;
        }
        const auto id_opt = HexCodec<UUID>::decode(id_ptr->as_string());
        if (!id_opt) {
            return std::nullopt;
        }
        return BaseModel::Metadata{*id_opt, version_ptr->as_uint64(),
                                   Timestamp{std::chrono::milliseconds{created_at_ptr->as_uint64()}}};
    }

    template<typename PasswordBinding>
    struct JsonCodec<UserIdentity<PasswordBinding>> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const UserIdentity<PasswordBinding> &user_identity) {
            boost::json::object json_object;
            encode_model_metadata(json_object, user_identity);
            json_object["username"] = user_identity.state.username;
            json_object["is_admin"] = user_identity.state.is_admin;
            json_object["password_binding"] =
                    boost::json::parse(JsonCodec<PasswordBinding>::encode(user_identity.state.password_binding));
            return boost::json::serialize(json_object);
        }

        static std::optional<UserIdentity<PasswordBinding>> decode(encoded_view data) {
            try {
                auto json_value = boost::json::parse(data);
                auto const *json_value_ptr = json_value.if_object();
                if (!json_value_ptr) {
                    return std::nullopt;
                }
                auto const *username_ptr = json_value_ptr->if_contains("username");
                auto const *password_binding_ptr = json_value_ptr->if_contains("password_binding");
                auto const *is_admin_ptr = json_value_ptr->if_contains("is_admin");
                if (!username_ptr || !is_admin_ptr || !password_binding_ptr) {
                    return std::nullopt;
                }
                auto password_binding_opt =
                        JsonCodec<PasswordBinding>::decode(boost::json::serialize(*password_binding_ptr));
                if (!password_binding_opt) {
                    return std::nullopt;
                }
                typename UserIdentity<PasswordBinding>::State state{std::string(username_ptr->as_string()),
                                                                    std::move(*password_binding_opt),
                                                                    is_admin_ptr->as_bool()};
                if (auto metadata = decode_model_metadata(*json_value_ptr)) {
                    return UserIdentity<PasswordBinding>::rehydrate(*metadata, std::move(state));
                }
                return UserIdentity<PasswordBinding>::create(std::move(state));
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<>
    struct JsonCodec<Author> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const Author &author) {
            boost::json::object json_object;
            encode_model_metadata(json_object, author);
            json_object["display_name"] = author.state.display_name;
            json_object["bio"] = author.state.bio;
            return boost::json::serialize(json_object);
        }

        static std::optional<Author> decode(encoded_view data) {
            try {
                auto json_value = boost::json::parse(data);
                auto const *json_value_ptr = json_value.if_object();
                if (!json_value_ptr) {
                    return std::nullopt;
                }
                auto const *display_name_ptr = json_value_ptr->if_contains("display_name");
                auto const *bio_ptr = json_value_ptr->if_contains("bio");
                if (!display_name_ptr || !bio_ptr) {
                    return std::nullopt;
                }
                Author::State state{std::string(display_name_ptr->as_string()), std::string(bio_ptr->as_string())};
                if (auto metadata = decode_model_metadata(*json_value_ptr)) {
                    return Author::rehydrate(*metadata, std::move(state));
                }
                return std::nullopt;
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<>
    struct JsonCodec<Post> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const Post &post) {
            boost::json::object json_object;
            encode_model_metadata(json_object, post);
            json_object["author_id"] = HexCodec<UUID>::encode(post.state.author_id);
            json_object["title"] = post.state.title;
            json_object["content"] = post.state.content;
            return boost::json::serialize(json_object);
        }

        static std::optional<Post> decode(encoded_view data) {
            try {
                auto json_value = boost::json::parse(data);
                auto const *json_value_ptr = json_value.if_object();
                if (!json_value_ptr) {
                    return std::nullopt;
                }
                auto const *author_id_ptr = json_value_ptr->if_contains("author_id");
                auto const *title_ptr = json_value_ptr->if_contains("title");
                auto const *content_ptr = json_value_ptr->if_contains("content");
                if (!author_id_ptr || !title_ptr || !content_ptr) {
                    return std::nullopt;
                }
                const auto author_id_opt = HexCodec<UUID>::decode(author_id_ptr->as_string());
                if (!author_id_opt) {
                    return std::nullopt;
                }
                Post::State state{*author_id_opt, std::string(title_ptr->as_string()),
                                  std::string(content_ptr->as_string())};
                if (auto metadata = decode_model_metadata(*json_value_ptr)) {
                    return Post::rehydrate(*metadata, std::move(state));
                }
                return Post::create(std::move(state));
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    static_assert(Codec<JsonCodec<UserIdentity<security::Argon2idBinding>>, UserIdentity<security::Argon2idBinding>>);
    static_assert(Codec<JsonCodec<UserIdentity<security::Sha256Binding>>, UserIdentity<security::Sha256Binding>>);
    static_assert(Codec<JsonCodec<Author>, Author>);
    static_assert(Codec<JsonCodec<Post>, Post>);

} // namespace mehara::prapancha::codec

#endif // PRAPANCHA_SERVER_CODEC_JSON_MODEL_CODEC_H_
