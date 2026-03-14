//
// Created by Aman Mehara on 03/02/26.
//

#ifndef PRAPANCHA_SERVER_MODEL_H_
#define PRAPANCHA_SERVER_MODEL_H_

#include <chrono>
#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <prapancha/security/hasher.h>
#include <prapancha/server/uuid.h>

namespace mehara::prapancha {

    using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

    struct increment_version_t {
        explicit increment_version_t() = default;
    };

    inline constexpr increment_version_t increment_version{};

    struct BaseModel {
        struct Metadata {
            UUID id;
            std::uint64_t version;
            Timestamp created_at;
        };

    private:
        Metadata metadata_;

    public:
        [[nodiscard]] const Metadata &metadata() const noexcept { return metadata_; }
        [[nodiscard]] const UUID &id() const noexcept { return metadata_.id; }
        [[nodiscard]] std::uint64_t version() const noexcept { return metadata_.version; }
        [[nodiscard]] Timestamp created_at() const noexcept { return metadata_.created_at; }

        BaseModel &operator=(const BaseModel &) = default;
        BaseModel &operator=(BaseModel &&) noexcept = default;

    protected:
        explicit BaseModel(const UUID uuid) :
            metadata_{uuid, 1, Timestamp{std::chrono::milliseconds{uuid.timestamp_ms()}}} {}

        BaseModel(const BaseModel &) = default;
        BaseModel(BaseModel &&) noexcept = default;

        explicit BaseModel(const BaseModel &other, increment_version_t) :
            metadata_{other.metadata_.id, other.metadata_.version + 1, other.metadata_.created_at} {}

        explicit BaseModel(const Metadata &metadata) : metadata_(metadata) {}

        explicit BaseModel(const UUID uuid, const std::uint64_t version, const Timestamp created_at) :
            metadata_{uuid, version, created_at} {}
    };

    template<typename M>
    concept Model = std::derived_from<M, BaseModel> && requires {
        { M::model_name } -> std::convertible_to<std::string_view>;
        typename M::State;
    };

    template<typename PasswordBinding>
    struct UserIdentity : BaseModel {
        static constexpr std::string_view model_name = "user_identity";

        struct [[nodiscard]] Attestation {
            [[nodiscard]] const UUID &id() const { return id_; }

        private:
            friend struct UserIdentity;
            const UUID id_;
            explicit Attestation(const UUID &uuid) : id_(uuid) {}
        };

        struct State {
            std::string username;
            PasswordBinding password_binding;
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

        static UserIdentity rehydrate(const Metadata &metadata, State s) {
            return UserIdentity(metadata, std::move(s));
        }

        static UserIdentity rehydrate(const UUID id, const uint64_t v, const Timestamp ts, State s) {
            return UserIdentity(id, v, ts, std::move(s));
        }

        [[nodiscard]] Attestation certify() const { return Attestation{this->id()}; }

    private:
        explicit UserIdentity(const UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        explicit UserIdentity(const UserIdentity &other, State s) :
            BaseModel(other, increment_version), state(std::move(s)) {}

        explicit UserIdentity(const Metadata &metadata, State s) : BaseModel(metadata), state(std::move(s)) {}

        explicit UserIdentity(const UUID id, const uint64_t v, const Timestamp ts, State s) :
            BaseModel(id, v, ts), state(std::move(s)) {}
    };

    struct Author : BaseModel {
        static constexpr std::string_view model_name = "author";

        struct State {
            std::string display_name;
            std::string bio;

            bool operator==(const State &) const = default;
        };

        const State state;

        template<typename PasswordBinding>
        static Author create(const UserIdentity<PasswordBinding>::Attestation attestation, State s) {
            return Author(attestation.id(), std::move(s));
        }

        [[nodiscard]] Author patch(State next_state) const {
            if (this->state == next_state) {
                return *this;
            }
            return Author(*this, std::move(next_state));
        }

        static Author rehydrate(const Metadata &metadata, State s) { return Author(metadata, std::move(s)); }

        static Author rehydrate(const UUID id, const uint64_t v, const Timestamp ts, State s) {
            return Author(id, v, ts, std::move(s));
        }

    private:
        explicit Author(const UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        explicit Author(const Author &other, State s) : BaseModel(other, increment_version), state(std::move(s)) {}

        explicit Author(const Metadata &metadata, State s) : BaseModel(metadata), state(std::move(s)) {}

        explicit Author(const UUID id, const uint64_t v, const Timestamp ts, State s) :
            BaseModel(id, v, ts), state(std::move(s)) {}
    };

    struct Post : BaseModel {
        static constexpr std::string_view model_name = "post";

        struct State {
            UUID author_id;
            std::string title;
            std::string content;

            bool operator==(const State &) const = default;
        };

        const State state;

        static Post create(State s) { return Post(UUID::generate(), std::move(s)); }

        static Post rehydrate(const Metadata &metadata, State s) { return Post(metadata, std::move(s)); }

        static Post rehydrate(const UUID id, const uint64_t v, const Timestamp ts, State s) {
            return Post(id, v, ts, std::move(s));
        }

        [[nodiscard]] Post patch(State next_state) const {
            if (this->state == next_state) {
                return *this;
            }
            return Post(*this, std::move(next_state));
        }

    private:
        explicit Post(const UUID id, State s) : BaseModel(id), state(std::move(s)) {}

        explicit Post(const Post &other, State s) : BaseModel(other, increment_version), state(std::move(s)) {}

        explicit Post(const Metadata &metadata, State s) : BaseModel(metadata), state(std::move(s)) {}

        explicit Post(const UUID id, const uint64_t v, const Timestamp ts, State s) :
            BaseModel(id, v, ts), state(std::move(s)) {}
    };


} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_MODEL_H_
