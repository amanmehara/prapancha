//
// Created by Aman Mehara on 08/03/26.
//

#ifndef PRAPANCHA_SERVER_CONTROLLER_ROOT_CONTROLLER_H_
#define PRAPANCHA_SERVER_CONTROLLER_ROOT_CONTROLLER_H_

#include <prapancha/server/controller/base_controller.h>

namespace mehara::prapancha {
    class RootController : public BaseController<RootController> {
    public:
        static constexpr std::string_view controller_name = "root";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            http::Response res{http::Status::Ok};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha!";
            sender(std::move(res));
        }
    };
} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_CONTROLLER_ROOT_CONTROLLER_H_
