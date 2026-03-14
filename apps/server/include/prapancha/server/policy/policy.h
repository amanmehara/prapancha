#ifndef PRAPANCHA_SERVER_POLICY_POLICY_H_
#define PRAPANCHA_SERVER_POLICY_POLICY_H_

#include <concepts>
#include <expected>
#include <string>
#include <tuple>

#include <prapancha/env/env.h>
#include <prapancha/server/http.h>
#include <prapancha/server/uuid.h>

namespace mehara::prapancha::policy {

    struct WithRequest {
        http::Request request;
    };

    struct WithIdentity {
        const UUID id;
        const std::string username;
        const std::string role;
    };

    struct WithValidation {};
    struct WithAdminAttestation {};
    struct WithStaffAttestation {};

    template<typename... Traits>
    using Context = env::Env<Traits...>;

    template<typename T, typename NewTrait>
    using Refined = env::Augmented<T, NewTrait>;

    template<typename T>
    using Result = std::expected<T, http::Status>;

    template<typename T>
    concept IsAuthorizationAttestation = std::same_as<T, WithAdminAttestation> || std::same_as<T, WithStaffAttestation>;

    template<typename T>
    concept HasRequest = requires(T v) { [](http::Request &) {}(v.request); };

    template<typename T>
    concept HasRole = requires(T v) {
        { v.role } -> std::convertible_to<std::string>;
    };

    namespace internal {
        inline Result<WithIdentity> authenticate(const http::Request &request) {
            if constexpr (true) {
                return {WithIdentity{}};
            }
            return std::unexpected(http::Status::Unauthorized);
        }

        template<IsAuthorizationAttestation Attestation>
        Result<Attestation> authorize(const std::string &user_role, const std::string_view required_role) {
            if (user_role == required_role) {
                return {Attestation{}};
            }
            return std::unexpected(http::Status::Forbidden);
        }
    } // namespace internal

    template<typename Trait>
    struct PolicyFor;

    template<>
    struct PolicyFor<WithRequest> {
        static Result<Context<WithRequest>> execute(auto &&req) {
            return Context<WithRequest>{WithRequest{std::forward<decltype(req)>(req)}};
        }
    };

    template<>
    struct PolicyFor<WithIdentity> {
        template<HasRequest T>
        static Result<Refined<T, WithIdentity>> execute(T &&ctx) {
            // Capture ctx by move into the lambda to ensure it stays valid
            // through the asynchronous/monadic transform.
            return internal::authenticate(ctx.request)
                    .transform([ctx = std::forward<T>(ctx)](WithIdentity &&id) mutable {
                        return Refined<T, WithIdentity>{std::move(ctx), std::move(id)};
                    });
        }
    };

    template<>
    struct PolicyFor<WithValidation> {
        template<typename T>
        static Result<Refined<T, WithValidation>> execute(T &&ctx) {
            return Refined<T, WithValidation>{std::forward<T>(ctx), WithValidation{}};
        }
    };

    template<IsAuthorizationAttestation Attestation>
    struct RolePolicy {
        template<HasRole T>
        static Result<Refined<T, Attestation>> execute(T &&ctx, std::string_view role) {
            return internal::authorize<Attestation>(ctx.role, role)
                    .transform([ctx = std::forward<T>(ctx)](Attestation &&att) mutable {
                        return Refined<T, Attestation>{std::move(ctx), std::move(att)};
                    });
        }
    };


    template<>
    struct PolicyFor<WithAdminAttestation> {
        static auto execute(auto &&ctx) {
            return RolePolicy<WithAdminAttestation>::execute(std::forward<decltype(ctx)>(ctx), "admin");
        }
    };

    template<>
    struct PolicyFor<WithStaffAttestation> {
        static auto execute(auto &&ctx) {
            return RolePolicy<WithStaffAttestation>::execute(std::forward<decltype(ctx)>(ctx), "staff");
        }
    };

} // namespace mehara::prapancha::policy

#endif // PRAPANCHA_SERVER_POLICY_POLICY_H_
