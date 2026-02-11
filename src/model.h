//
// Created by Aman Mehara on 03/02/26.
//

#ifndef PRAPANCHA_MODEL_H
#define PRAPANCHA_MODEL_H

#include <chrono>
#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include "uuid.h"

namespace mehara::prapancha {

    using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

    struct BaseModel {
        const UUID id;
        const uint64_t version;
        const Timestamp created_at;

    protected:
        explicit BaseModel(const UUID uuid) :
            id(uuid), version(1), created_at(std::chrono::milliseconds{id.timestamp_ms()}) {}

        explicit BaseModel(const BaseModel &other) :
            id(other.id), version(other.version + 1), created_at(std::chrono::milliseconds{id.timestamp_ms()}) {}
    };


    template<typename M>
    concept Model = std::derived_from<M, BaseModel> && requires {
        { M::ModelName } -> std::convertible_to<std::string_view>;
        typename M::State;
    };

    struct Author : BaseModel {
        static constexpr std::string_view ModelName = "author";

        struct State {
            std::string display_name;
            std::string bio;

            bool operator==(const State &) const = default;
        };

        const State state;

        static Author create(State s) { return {UUID::generate(), std::move(s)}; }

        [[nodiscard]] Author patch(State next_state) const {
            if (this->state == next_state) {
                return *this;
            }
            return {*this, std::move(next_state)};
        }

    private:
        Author(const UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        Author(const Author &other, State s) : BaseModel(other), state(std::move(s)) {}
    };

    struct Post : BaseModel {
        static constexpr std::string_view ModelName = "post";

        struct State {
            UUID author_id;
            std::string title;
            std::string content;

            bool operator==(const State &) const = default;
        };

        const State state;

        static Post create(State s) { return {UUID::generate(), std::move(s)}; }

        [[nodiscard]] Post patch(State next_state) const {
            if (this->state == next_state) {
                return *this;
            }
            return {*this, std::move(next_state)};
        }

    private:
        Post(UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        Post(const Post &other, State s) : BaseModel(other), state(std::move(s)) {}
    };

    static_assert(Model<Author>, "Author does not satisfy the Model concept.");
    static_assert(Model<Post>, "Post does not satisfy the Model concept.");

} // namespace mehara::prapancha

#endif // PRAPANCHA_MODEL_H
