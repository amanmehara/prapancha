//
// Created by Aman Mehara on 08/03/26.
//

#include <prapancha/server/beast_adapter.h>

namespace mehara::prapancha::http {

    Method from_beast(const boost::beast::http::verb beast_verb) noexcept {
        switch (beast_verb) {
            case boost::beast::http::verb::delete_:
                return Method::Delete;
            case boost::beast::http::verb::get:
                return Method::Get;
            case boost::beast::http::verb::head:
                return Method::Head;
            case boost::beast::http::verb::options:
                return Method::Options;
            case boost::beast::http::verb::patch:
                return Method::Patch;
            case boost::beast::http::verb::post:
                return Method::Post;
            case boost::beast::http::verb::put:
                return Method::Put;
            default:
                return Method::Unknown;
        }
    }

    Request from_beast(boost::beast::http::request<boost::beast::http::vector_body<uint8_t>> &beast_request) {
        Request request;
        request.target = std::string(beast_request.target());
        request.method = from_beast(beast_request.method());
        request.headers.reserve(std::distance(beast_request.begin(), beast_request.end()));
        for (const auto &field: beast_request) {
            request.headers.push_back({std::string(field.name_string()), std::string(field.value())});
        }
        request.body = std::move(beast_request.body());
        return request;
    }

    boost::beast::http::response<boost::beast::http::string_body> to_beast(Response &&response) {
        boost::beast::http::response<boost::beast::http::string_body> beast_response;
        beast_response.version(11);
        beast_response.result(static_cast<boost::beast::http::status>(response.status));
        for (const auto &[name, value]: response.headers) {
            beast_response.set(name, value);
        }
        beast_response.body() = std::move(response.body);
        beast_response.prepare_payload();
        return beast_response;
    }

} // namespace mehara::prapancha::http
