//
// Created by Aman Mehara on 14/03/26.
//

#ifndef PRAPANCHA_SERVER_PERSISTENCE_REGISTRY_H_
#define PRAPANCHA_SERVER_PERSISTENCE_REGISTRY_H_

#include <memory>
#include <variant>

#include <prapancha/security/hasher.h>
#include <prapancha/server/codec/json_codec.h>
#include <prapancha/server/codec/json_model_codec.h>
#include <prapancha/server/model.h>

#include <prapancha/server/persistence/persistence.h>

namespace mehara::prapancha {

    using UserIdentityPersistence =
            std::variant<FilePersistence<UserIdentity<security::Argon2id>,
                                         codec::JsonCodec<UserIdentity<security::Argon2id>>>,
                         FilePersistence<UserIdentity<security::Sha256>,
                                         codec::JsonCodec<UserIdentity<security::Sha256>>>>;

    struct PersistenceRegistry {
        inline static std::unique_ptr<UserIdentityPersistence> user_identity_persistence;

        template<typename PasswordBinding>
        static void initialize_user_identity(const std::string &file_path) {
            using TargetPersistence =
                    FilePersistence<UserIdentity<PasswordBinding>, codec::JsonCodec<UserIdentity<PasswordBinding>>>;
            user_identity_persistence =
                    std::make_unique<UserIdentityPersistence>(std::in_place_type<TargetPersistence>, file_path);
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_PERSISTENCE_REGISTRY_H_
