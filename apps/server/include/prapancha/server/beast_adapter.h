//
// Created by Aman Mehara on 08/03/26.
//

#ifndef PRAPANCHA_SERVER_BEAST_ADAPTER_H_
#define PRAPANCHA_SERVER_BEAST_ADAPTER_H_

#include <boost/beast/http.hpp>
#include <prapancha/server/http.h>

namespace mehara::prapancha::http {

    [[nodiscard]] Method from_beast(boost::beast::http::verb beast_verb) noexcept;

    [[nodiscard]] Request
    from_beast(boost::beast::http::request<boost::beast::http::vector_body<uint8_t>> &beast_request);

    [[nodiscard]] boost::beast::http::response<boost::beast::http::string_body> to_beast(Response &&response);

} // namespace mehara::prapancha::http

#endif // PRAPANCHA_SERVER_BEAST_ADAPTER_H_
