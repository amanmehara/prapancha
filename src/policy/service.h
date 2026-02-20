//
// Created by Aman Mehara on 18/02/26.
//

#ifndef PRAPANCHA_POLICY_SERVICE_H
#define PRAPANCHA_POLICY_SERVICE_H

#include "context.h"

#include <expected>

#include "drogon/HttpRequest.h"
#include "drogon/HttpResponse.h"

namespace mehara::prapancha::policy {

    namespace internal {
        Result<WithIdentity> authenticate(const drogon::HttpRequestPtr &req);

        template<IsAuthorizationAttestation Attestation>
        Result<Attestation> authorize(const std::string &userRole, std::string_view requiredRole) {
            if (userRole == requiredRole) {
                return Attestation{};
            }
            return std::unexpected(drogon::HttpResponse::newHttpResponse(drogon::k403Forbidden, drogon::CT_TEXT_PLAIN));
        }
    } // namespace internal

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

#endif // PRAPANCHA_POLICY_SERVICE_H
