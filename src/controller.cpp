//
// Created by Aman Mehara on 01/02/26.
//

#include "controller.h"

#include <drogon/drogon.h>

namespace mehara::prapancha {

    void RootController::handle(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
        const auto response = drogon::HttpResponse::newHttpResponse();
        response->setStatusCode(drogon::k200OK);
        response->setContentTypeCode(drogon::CT_TEXT_HTML);
        response->setBody("प्रपञ्च — Prapancha!");
        callback(response);
    }

} // namespace mehara::prapancha
