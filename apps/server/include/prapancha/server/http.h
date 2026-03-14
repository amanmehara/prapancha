//
// Created by Aman Mehara on 08/03/26.
//

#ifndef PRAPANCHA_SERVER_HTTP_H_
#define PRAPANCHA_SERVER_HTTP_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mehara::prapancha::http {

    enum class Method { Delete, Get, Head, Options, Patch, Post, Put, Unknown };

    struct MethodTraits {
        std::string_view name;
        bool is_safe;
        bool is_idempotent;
    };

    [[nodiscard]] constexpr MethodTraits get_traits(const Method method) noexcept {
        using enum Method;
        switch (method) {
            case Delete:
                return {"DELETE", false, true};
            case Get:
                return {"GET", true, true};
            case Head:
                return {"HEAD", true, true};
            case Options:
                return {"OPTIONS", true, true};
            case Patch:
                return {"PATCH", false, false};
            case Post:
                return {"POST", false, false};
            case Put:
                return {"PUT", false, true};
            default:
                return {"UNKNOWN", false, false};
        }
    }

    enum class Status {
        Ok = 200,
        Created = 201,
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        Conflict = 409,
        InternalServerError = 500,
        NotImplemented = 501
    };

    struct Header {
        std::string name;
        std::string value;
    };

    struct Request {
        Method method;
        std::string target;
        std::vector<Header> headers;
        std::vector<uint8_t> body;

        [[nodiscard]] std::optional<std::string> header(std::string_view name) const {
            std::string combined;
            bool found = false;
            for (const auto &h: headers) {
                if (h.name.size() == name.size() && strcasecmp(h.name.c_str(), name.data()) == 0) {
                    if (!combined.empty())
                        combined += ", ";
                    combined += h.value;
                    found = true;
                }
            }
            return found ? std::make_optional(std::move(combined)) : std::nullopt;
        }
    };

    struct Response {
        Status status = Status::Ok;
        std::vector<Header> headers;
        std::string body; // Using string for text-based responses

        void set_header(std::string name, std::string value) { headers.push_back({std::move(name), std::move(value)}); }
    };

} // namespace mehara::prapancha::http

#endif // PRAPANCHA_SERVER_HTTP_H_
