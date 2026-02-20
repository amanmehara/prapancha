//
// Created by Aman Mehara on 20/02/26.
//

#ifndef PRAPANCHA_POLICY_MAPPING_H
#define PRAPANCHA_POLICY_MAPPING_H

#include "context.h"
#include "service.h"

namespace mehara::prapancha::policy {

    template<typename Trait>
    struct PolicyFor;

    template<>
    struct PolicyFor<WithRequest> {
        static auto execute(const drogon::HttpRequestPtr &req) { return initialize(req); }
    };

    template<>
    struct PolicyFor<WithIdentity> {
        static auto execute(auto &&ctx) { return authenticate(std::forward<decltype(ctx)>(ctx)); }
    };

    template<>
    struct PolicyFor<WithValidation> {
        static auto execute(auto &&ctx) { return validate(std::forward<decltype(ctx)>(ctx)); }
    };

    template<>
    struct PolicyFor<WithAdminAttestation> {
        static auto execute(auto &&ctx) {
            static_assert(IsAuthorizationAttestation<WithAdminAttestation>,
                          "Trait must be a valid Authorization Attestation");
            return authorize<WithAdminAttestation>(std::forward<decltype(ctx)>(ctx), "admin");
        }
    };

    template<>
    struct PolicyFor<WithStaffAttestation> {
        static auto execute(auto &&ctx) {
            static_assert(IsAuthorizationAttestation<WithStaffAttestation>,
                          "Trait must be a valid Authorization Attestation");
            return authorize<WithStaffAttestation>(std::forward<decltype(ctx)>(ctx), "staff");
        }
    };

} // namespace mehara::prapancha::policy

#endif // PRAPANCHA_POLICY_MAPPING_H
