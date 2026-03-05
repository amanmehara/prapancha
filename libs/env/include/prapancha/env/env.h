//
// Created by Aman Mehara on 05/03/26.
//

#ifndef PRAPANCHA_ENV_H
#define PRAPANCHA_ENV_H

#include <concepts>
#include <expected>
#include <system_error>
#include <type_traits>
#include <utility>

namespace mehara::prapancha::env {

    template<typename Capability, typename... Capabilities>
    struct is_unique : std::bool_constant<(!std::is_same_v<Capability, Capabilities> && ...)> {};

    template<typename... Capabilities>
    struct Env : Capabilities... {
        static_assert((is_unique<Capabilities, Capabilities...>::value && ...),
                      "Cannot create Env with duplicate capabilities.");

        explicit Env(Capabilities &&...capabilities) : Capabilities(std::forward<Capabilities>(capabilities))... {}

        template<typename... ExistingCapabilities>
        explicit Env(Env<ExistingCapabilities...> &&existing, Capabilities &&...deltas) :
            ExistingCapabilities(std::move(static_cast<ExistingCapabilities &&>(existing)))...,
            Capabilities(std::forward<Capabilities>(deltas))... {}
    };

    template<typename T>
    using Result = std::expected<T, std::error_code>;

    template<typename Current, typename NextCapability>
    struct Evolution;

    template<typename... Capabilities, typename NextCapability>
    struct Evolution<Env<Capabilities...>, NextCapability> {
        using type = Env<Capabilities..., NextCapability>;
    };

    template<typename Current, typename NextCapability>
    using Augmented = Evolution<std::remove_cvref_t<Current>, NextCapability>::type;

    template<typename... Capabilities1, typename... Capabilities2>
    auto fuse(Env<Capabilities1...> &&env1, Env<Capabilities2...> &&env2) {
        static_assert((is_unique<Capabilities2, Capabilities1...>::value && ...),
                      "Cannot fuse Envs with overlapping Capabilities.");
        return Env<Capabilities1..., Capabilities2...>(std::move(static_cast<Capabilities1 &&>(env1))...,
                                                       std::move(static_cast<Capabilities2 &&>(env2))...);
    }

    template<typename... Selected, typename... Existing>
    auto isolate(Env<Existing...> &existing) {
        static_assert((std::is_base_of_v<Selected, Env<Existing...>> && ...),
                      "Cannot isolate missing capabilities from Env.");
        return Env<Selected...>(static_cast<Selected &>(existing)...);
    }

    template<typename T>
    struct CapabilityFor;

    template<typename NextCapability, typename Current, typename... Args>
        requires requires(Args &&...args) {
            {
                CapabilityFor<NextCapability>::execute(std::forward<Args>(args)...)
            } -> std::same_as<Result<NextCapability>>;
        }
    auto evolve(Current &&env, Args &&...args) -> Result<Augmented<Current, NextCapability>> {
        return CapabilityFor<NextCapability>::execute(std::forward<Args>(args)...)
                .transform([env = std::forward<Current>(env)](NextCapability &&capability) mutable {
                    return Augmented<Current, NextCapability>(std::move(env), std::move(capability));
                });
    }

} // namespace mehara::prapancha::env

#endif // PRAPANCHA_ENV_H
