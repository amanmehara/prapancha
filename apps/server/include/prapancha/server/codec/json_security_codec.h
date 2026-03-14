//
// Created by Aman Mehara on 13/03/26.
//

#ifndef PRAPANCHA_SERVER_CODEC_JSON_SECURITY_CODEC_H_
#define PRAPANCHA_SERVER_CODEC_JSON_SECURITY_CODEC_H_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <boost/json.hpp>

#include <prapancha/security/hasher.h>
#include <prapancha/server/codec/codec.h>
#include <prapancha/server/codec/hex_codec.h>
#include <prapancha/server/codec/json_codec.h>

namespace mehara::prapancha::codec {

    template<>
    struct JsonCodec<security::Argon2idBinding> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const security::Argon2idBinding &binding) {
            boost::json::object json_object;
            json_object["v"] = binding.version;
            json_object["m"] = binding.m;
            json_object["t"] = binding.t;
            json_object["p"] = binding.p;
            json_object["salt"] = HexCodec<std::vector<std::uint8_t>>::encode(binding.salt);
            json_object["hash"] = HexCodec<std::vector<std::uint8_t>>::encode(binding.hash);
            return boost::json::serialize(json_object);
        }

        static std::optional<security::Argon2idBinding> decode(encoded_view data) {
            try {
                auto json_object = boost::json::parse(data).as_object();
                auto salt = HexCodec<std::vector<std::uint8_t>>::decode(
                        std::string_view(json_object.at("salt").as_string()));
                auto hash = HexCodec<std::vector<std::uint8_t>>::decode(
                        std::string_view(json_object.at("hash").as_string()));
                if (!salt || !hash) {
                    return std::nullopt;
                }
                return security::Argon2idBinding{static_cast<std::uint32_t>(json_object.at("v").as_int64()),
                                                 static_cast<std::uint32_t>(json_object.at("m").as_int64()),
                                                 static_cast<std::uint32_t>(json_object.at("t").as_int64()),
                                                 static_cast<std::uint32_t>(json_object.at("p").as_int64()),
                                                 std::move(*salt),
                                                 std::move(*hash)};
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    template<>
    struct JsonCodec<security::Sha256Binding> {
        using encoded_type = std::string;
        using encoded_view = std::string_view;

        static encoded_type encode(const security::Sha256Binding &binding) {
            boost::json::object json_object;
            json_object["hash"] = HexCodec<std::vector<std::uint8_t>>::encode(binding.hash);
            return boost::json::serialize(json_object);
        }

        static std::optional<security::Sha256Binding> decode(encoded_view data) {
            try {
                auto json_object = boost::json::parse(data).as_object();
                auto hash = HexCodec<std::vector<std::uint8_t>>::decode(
                        std::string_view(json_object.at("hash").as_string()));
                if (!hash) {
                    return std::nullopt;
                }
                return security::Sha256Binding{std::move(*hash)};
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    static_assert(Codec<JsonCodec<security::Argon2idBinding>, security::Argon2idBinding>);
    static_assert(Codec<JsonCodec<security::Sha256Binding>, security::Sha256Binding>);

} // namespace mehara::prapancha::codec

#endif // PRAPANCHA_SERVER_CODEC_JSON_SECURITY_CODEC_H_
