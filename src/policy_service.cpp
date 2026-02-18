//
// Created by Aman Mehara on 18/02/26.
//

#include "policy_service.h"

#include <expected>

namespace mehara::prapancha::policy::internal {

    Result<WithIdentity> authenticate(const drogon::HttpRequestPtr &req) {
        auto session = req->session();
        if (session && session->find("user_id")) {
            return WithIdentity{session->get<std::string>("user_id"), session->get<std::string>("role")};
        }
        return std::unexpected(drogon::HttpResponse::newHttpResponse(drogon::k401Unauthorized, drogon::CT_TEXT_PLAIN));
    }

    Result<void> authorize(const std::string &userRole, std::string_view requiredRole) {
        if (requiredRole == "any" || userRole == requiredRole) {
            return {};
        }
        return std::unexpected(drogon::HttpResponse::newHttpResponse(drogon::k403Forbidden, drogon::CT_TEXT_PLAIN));
    }

} // namespace mehara::prapancha::policy::internal
