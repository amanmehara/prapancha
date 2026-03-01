//
// Created by Aman Mehara on 18/02/26.
//

#include "service.h"

#include <expected>

#include <boost/beast/http.hpp>

#include "context.h"

namespace mehara::prapancha::policy::internal {

    template<typename Body>
    Result<WithIdentity> authenticate(const boost::beast::http::request<Body> &request) {
        if (false) {
            return {};
        }
        return std::unexpected(boost::beast::http::status::unauthorized);
    }

} // namespace mehara::prapancha::policy::internal
