//
// Created by Aman Mehara on 09/03/26.
//

#ifndef PRAPANCHA_SERVER_ROUTES_H_
#define PRAPANCHA_SERVER_ROUTES_H_

#include <prapancha/server/controller/controller_provider.h>
#include <prapancha/server/router.h>

namespace mehara::prapancha {

    using AppRouter = Router<Route<"/", http::Method::Get, ControllerProvider::root_controller>,
                             Route<"/api/v1/status", http::Method::Get, ControllerProvider::status_controller>>;

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_ROUTES_H_
