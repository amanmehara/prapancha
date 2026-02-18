//
// Created by Aman Mehara on 18/02/26.
//

#ifndef PRAPANCHA_CONTEXT_H
#define PRAPANCHA_CONTEXT_H

#include "drogon/HttpRequest.h"
#include "drogon/HttpResponse.h"

#include <concepts>
#include <expected>
#include <string>

namespace mehara::prapancha::policy {

    struct WithRequest {
        drogon::HttpRequestPtr request;
    };

    struct WithIdentity {
        const std::string userId;
        const std::string role;
    };

    struct WithValidation {
        const bool isSanitized = true;
    };

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
    using Result = std::expected<T, drogon::HttpResponsePtr>;

    template<typename T>
    concept HasRequest = requires(T v) {
        { v.request } -> std::convertible_to<drogon::HttpRequestPtr>;
    };
    template<typename T>
    concept HasIdentity = requires(T v) {
        { v.userId } -> std::convertible_to<std::string>;
    };
    template<typename T>
    concept HasRole = requires(T v) {
        { v.role } -> std::convertible_to<std::string>;
    };

} // namespace mehara::prapancha::policy

#endif // PRAPANCHA_CONTEXT_H
