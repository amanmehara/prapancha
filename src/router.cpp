//
// Created by Aman Mehara on 01/02/26.
//

#include "router.h"

namespace mehara::prapancha {

    template<typename T>
    auto Router::toHandler(const std::shared_ptr<T> &controller) {
        static_assert(std::is_base_of_v<BaseController<T>, T>,
                      "The provided class must inherit from BaseController<T>.");
        return [controller](const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            controller->dispatch(request, std::move(callback));
        };
    }

    void Router::configure(drogon::HttpAppFramework &app) {
        auto rootController = std::make_shared<RootController>();
        app.registerHandler("/", toHandler(rootController), {drogon::Get});
    }

} // namespace mehara::prapancha
