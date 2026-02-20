//
// Created by Aman Mehara on 18/02/26.
//

#include "service.h"

#include <expected>

#include "drogon/HttpRequest.h"
#include "drogon/HttpResponse.h"

#include "context.h"

namespace mehara::prapancha::policy::internal {

    Result<WithIdentity> authenticate(const drogon::HttpRequestPtr &req) {
        auto session = req->session();
        if (session && session->find("user_id")) {
            return WithIdentity{session->get<std::string>("user_id"), session->get<std::string>("role")};
        }
        return std::unexpected(drogon::HttpResponse::newHttpResponse(drogon::k401Unauthorized, drogon::CT_TEXT_PLAIN));
    }

} // namespace mehara::prapancha::policy::internal
