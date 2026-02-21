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
        if (const auto session = req->session(); session && session->find("identity")) {
            return session->get<WithIdentity>("identity");
        }
        return std::unexpected(drogon::HttpResponse::newHttpResponse(drogon::k401Unauthorized, drogon::CT_TEXT_PLAIN));
    }

} // namespace mehara::prapancha::policy::internal
