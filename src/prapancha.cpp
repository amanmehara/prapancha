//
// Created by Aman Mehara on 01/02/26.
//

#include "prapancha.h"

#include "configuration.h"
#include "router.h"

#include <drogon/drogon.h>

namespace mehara::prapancha {

    void run(int argc, char *argv[]) {
        configuration::initialize(configuration::from_cli(argc, argv));
        auto &app = drogon::app();
        auto router = std::make_shared<Router>(app, configuration::Active);
        const auto &host = configuration::Active->network.host;
        const auto &port = configuration::Active->network.port;
        const auto &thread_count = configuration::Active->network.thread_count;
        LOG_INFO << "प्रपञ्च — Prapancha starting on http://" << host << ":" << port;
        app.enableSession(1200, drogon::Cookie::SameSite::kLax)
                .addListener(host, port)
                .setThreadNum(thread_count)
                .run();
    }

} // namespace mehara::prapancha
