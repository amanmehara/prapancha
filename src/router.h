//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_ROUTER_H
#define PRAPANCHA_ROUTER_H
#include <memory>

#include "controller.h"

namespace mehara::prapancha {

    class Router {
    public:
        static void configure(drogon::HttpAppFramework &app);

    private:
        template<typename T>
        auto static toHandler(const std::shared_ptr<T> &controller);
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_ROUTER_H
