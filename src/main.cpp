//
// Created by Aman Mehara on 31/01/26.
//

#include <drogon/drogon.h>

using namespace drogon;

int main() {

    app().registerHandler("/", [](const HttpRequestPtr &_, std::function<void(const HttpResponsePtr &)> &&callback) {
        const auto response = HttpResponse::newHttpResponse();
        response->setStatusCode(k200OK);
        response->setContentTypeCode(CT_TEXT_HTML);
        response->setBody("प्रपञ्च — Prapancha!");
        callback(response);
    });

    const auto &ip = "127.0.0.1";
    const auto &port = 8080;

    LOG_INFO << "प्रपञ्च — Prapancha starting on http://" << ip << ":" << port;

    app().addListener(ip, port).setThreadNum(0).run();

    return 0;
}
