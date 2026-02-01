//
// Created by Aman Mehara on 01/02/26.
//

#include "prapancha.h"
#include "router.h"

#include <drogon/drogon.h>

namespace mehara::prapancha {

    void run() {
        auto &app = drogon::app();
        Router::configure(app);
        const auto &ip = "127.0.0.1";
        const auto &port = 8080;
        LOG_INFO << "प्रपञ्च — Prapancha starting on http://" << ip << ":" << port;
        app.addListener(ip, port).setThreadNum(0).run();
    }

} // namespace mehara::prapancha
