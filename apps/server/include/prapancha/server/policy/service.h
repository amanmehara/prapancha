//
// Created by Aman Mehara on 18/02/26.
//

#ifndef PRAPANCHA_SERVER_POLICY_SERVICE_H
#define PRAPANCHA_SERVER_POLICY_SERVICE_H

#include "context.h"

#include <expected>

#include <boost/beast/http.hpp>

namespace mehara::prapancha::policy {

    namespace internal {
        template<typename Body>
        Result<WithIdentity> authenticate(const boost::beast::http::request<Body> &request);

        template<IsAuthorizationAttestation Attestation>
        Result<Attestation> authorize(const std::string &userRole, const std::string_view requiredRole) {
            if (userRole == requiredRole) {
                return Attestation{};
            }
            return std::unexpected(boost::beast::http::status::forbidden);
        }
    } // namespace internal

    template<typename Body>
    Result<Context<WithRequest<Body>>> initialize(const boost::beast::http::request<Body> &request) {
        return Context<WithRequest<Body>>(WithRequest<Body>{request});
    }

    template<HasRequest T>
    Result<Refined<T, WithIdentity>> authenticate(T &&ctx) {
        auto res = internal::authenticate(ctx.request);
        if (!res)
            return std::unexpected(res.error());
        return Refined<T, WithIdentity>(std::forward<T>(ctx), std::move(res.value()));
    }

    template<IsAuthorizationAttestation Attestation, HasRole T>
    Result<Refined<T, Attestation>> authorize(T &&ctx, std::string_view requiredRole) {
        auto res = internal::authorize<Attestation>(ctx.role, requiredRole);
        if (!res)
            return std::unexpected(res.error());
        return std::forward<T>(ctx, std::move(res.value()));
    }

    template<typename T>
    Result<Refined<T, WithValidation>> validate(T &&ctx) {
        return Refined<T, WithValidation>(std::forward<T>(ctx), WithValidation{true});
    }

} // namespace mehara::prapancha::policy

#endif // PRAPANCHA_SERVER_POLICY_SERVICE_H
