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

    // The tag type
    struct increment_version_t {
        explicit increment_version_t() = default;
    };
    // The tag instance used for dispatch
    inline constexpr increment_version_t increment_version{};

    struct BaseModel {
        const UUID id;
        const uint64_t version;
        const Timestamp created_at;

    protected:
        explicit BaseModel(const UUID uuid) :
            id(uuid), version(1), created_at(std::chrono::milliseconds{id.timestamp_ms()}) {}

        BaseModel(const BaseModel &) = default;
        BaseModel(BaseModel &&) noexcept = default;

        explicit BaseModel(const BaseModel &other, increment_version_t) :
            id(other.id), version(other.version + 1), created_at(other.created_at) {}

        explicit BaseModel(const UUID uuid, const uint64_t v, const Timestamp ts) :
            id(uuid), version(v), created_at(ts) {}
    };


    template<typename M>
    concept Model = std::derived_from<M, BaseModel> && requires {
        { M::ModelName } -> std::convertible_to<std::string_view>;
        typename M::State;
    };

    struct UserIdentity : BaseModel {
        static constexpr std::string_view ModelName = "user_identity";

        struct Attestation {
            [[nodiscard]] const UUID &id() const { return _id; }

        private:
            friend struct UserIdentity;
            const UUID _id;
            explicit Attestation(const UUID &uuid) : _id(uuid) {}
        };

        struct State {
            std::string email;
            std::string password_hash;
            bool is_admin;

            bool operator==(const State &) const = default;
        };

        const State state;

        static UserIdentity create(State s) { return UserIdentity(UUID::generate(), std::move(s)); }

        [[nodiscard]] UserIdentity patch(State next_state) const {
            if (this->state == next_state) {
                return *this;
            }
            return UserIdentity(*this, std::move(next_state));
        }

        static UserIdentity rehydrate(const UUID id, const uint64_t v, const Timestamp ts, State s) {
            return UserIdentity(id, v, ts, std::move(s));
        }

        [[nodiscard]] Attestation certify() const {
            return Attestation{this->id};
        }

    private:
        explicit UserIdentity(const UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        explicit UserIdentity(const UserIdentity &other, State s) :
            BaseModel(other, increment_version), state(std::move(s)) {}

        explicit UserIdentity(const UUID id, const uint64_t v, const Timestamp ts, State s) :
            BaseModel(id, v, ts), state(std::move(s)) {}
    };

    struct Author : BaseModel {
        static constexpr std::string_view ModelName = "author";

        struct State {
            std::string display_name;
            std::string bio;

            bool operator==(const State &) const = default;
        };

        const State state;

        static Author create(const UserIdentity::Attestation attestation, State s) {
            return Author(attestation.id(), std::move(s));
        }

        [[nodiscard]] Author patch(State next_state) const {
            if (this->state == next_state) {
                return *this;
            }
            return Author(*this, std::move(next_state));
        }

        static Author rehydrate(const UUID id, const uint64_t v, const Timestamp ts, State s) {
            return Author(id, v, ts, std::move(s));
        }

    private:
        explicit Author(const UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        explicit Author(const Author &other, State s) : BaseModel(other, increment_version), state(std::move(s)) {}

        explicit Author(const UUID id, const uint64_t v, const Timestamp ts, State s) :
            BaseModel(id, v, ts), state(std::move(s)) {}
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

        static Post create(State s) { return Post(UUID::generate(), std::move(s)); }

        static Post rehydrate(const UUID id, const uint64_t v, const Timestamp ts, State s) {
            return Post(id, v, ts, std::move(s));
        }

        [[nodiscard]] Post patch(State next_state) const {
            if (this->state == next_state) {
                return *this; // Passive copy: version remains same
            }
            return Post(*this, std::move(next_state));
        }

    private:
        explicit Post(const UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        explicit Post(const Post &other, State s) : BaseModel(other, increment_version), state(std::move(s)) {}

        explicit Post(const UUID id, const uint64_t v, const Timestamp ts, State s) :
            BaseModel(id, v, ts), state(std::move(s)) {}
    };

    static_assert(Model<Author>, "Author does not satisfy the Model concept.");
    static_assert(Model<Post>, "Post does not satisfy the Model concept.");
    static_assert(Model<UserIdentity>, "UserIdentity does not satisfy the Model concept.");

} // namespace mehara::prapancha

#endif // PRAPANCHA_MODEL_H
