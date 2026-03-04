//
// Created by Aman Mehara on 18/02/26.
//

#ifndef PRAPANCHA_SERVER_POLICY_CONTEXT_H
#define PRAPANCHA_SERVER_POLICY_CONTEXT_H

#include <concepts>
#include <expected>
#include <string>

#include <boost/beast/http.hpp>

#include "../uuid.h"

namespace mehara::prapancha::policy {

    template<typename Body>
    struct WithRequest {
        boost::beast::http::request<Body> request;
    };

    struct WithIdentity {
        const UUID id;
        const std::string username;
        const std::string role;
    };

    struct WithValidation {
        const bool isSanitized = true;
    };

    struct WithAdminAttestation {};

    struct WithStaffAttestation {};

    template<typename... NewTraits>
    struct Context : NewTraits... {
        explicit Context(NewTraits &&...traits) : NewTraits(std::forward<NewTraits>(traits))... {}

        template<typename... ExistingTraits>
        explicit Context(Context<ExistingTraits...> &&existing, NewTraits &&...deltas) :
            ExistingTraits(std::move(static_cast<ExistingTraits &&>(existing)))...,
            NewTraits(std::forward<NewTraits>(deltas))... {}
    };

    template<typename CurrentContext, typename NewTrait>
    struct Refiner;

    template<typename... Traits, typename NewTrait>
    struct Refiner<Context<Traits...>, NewTrait> {
        using type = Context<Traits..., NewTrait>;
    };

    template<typename T, typename NewTrait>
    using Refined = Refiner<T, NewTrait>::type;

    template<typename T>
    using Result = std::expected<T, boost::beast::http::status>;

    template<typename T>
    concept IsAuthorizationAttestation = std::same_as<T, WithAdminAttestation> || std::same_as<T, WithStaffAttestation>;

    template<typename T>
    concept HasRequest = requires(T v) { []<typename Body>(boost::beast::http::request<Body> &) {}(v.request); };
    template<typename T>
    concept HasIdentity = requires(T v) {
        { v.id } -> std::convertible_to<UUID>;
        { v.username } -> std::convertible_to<std::string>;
    };
    template<typename T>
    concept HasRole = requires(T v) {
        { v.role } -> std::convertible_to<std::string>;
    };

    template<typename T>
    concept IsAdmin = std::derived_from<T, WithAdminAttestation>;

    template<typename T>
    concept IsStaff = std::derived_from<T, WithStaffAttestation>;

} // namespace mehara::prapancha::policy

#endif // PRAPANCHA_SERVER_POLICY_CONTEXT_H
