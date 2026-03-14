//
// Created by Aman Mehara on 08/03/26.
//

#ifndef PRAPANCHA_SERVER_CONTROLLER_VOID_CONTROLLER_H_
#define PRAPANCHA_SERVER_CONTROLLER_VOID_CONTROLLER_H_

#include <prapancha/server/controller/base_controller.h>

namespace mehara::prapancha {

    class VoidController : public BaseController<VoidController> {
    public:
        static constexpr std::string_view ControllerName = "void";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            Loggers::App().log_warn("प्रपञ्च — Prapancha: Void → [{} {}]", http::get_traits(ctx.request.method).name,
                                    ctx.request.target);
            http::Response res{http::Status::NotFound};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha: शून्यम्। Nihil!";
            sender(std::move(res));
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_CONTROLLER_VOID_CONTROLLER_H_
