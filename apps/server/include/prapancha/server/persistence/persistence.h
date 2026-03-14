//
// Created by Aman Mehara on 03/02/26.
//

#ifndef PRAPANCHA_SERVER_PERSISTENCE_PERSISTENCE_H_
#define PRAPANCHA_SERVER_PERSISTENCE_PERSISTENCE_H_

#include <filesystem>
#include <fstream>

#include <prapancha/server/codec/codec.h>
#include <prapancha/server/codec/hex_codec.h>
#include <prapancha/server/model.h>
#include <prapancha/server/uuid.h>

namespace mehara::prapancha {

    template<typename P, typename M>
    concept Persistence = requires(P policy, const M &model, const UUID &id) {
        requires Model<M>;
        { policy.save(model) } -> std::same_as<void>;
        { policy.load(id) } -> std::same_as<std::optional<M>>;
        { policy.all() } -> std::same_as<std::vector<M>>;
        { policy.remove(id) } -> std::same_as<bool>;
    };

    template<typename M, typename C>
        requires codec::Codec<C, M> && std::same_as<typename C::EncodedType, std::string>
    class FilePersistence {
    public:
        explicit FilePersistence(std::filesystem::path path) : directory_(std::move(path)) {
            if (!std::filesystem::exists(directory_)) {
                std::filesystem::create_directories(directory_);
            }
        }

        void save(const M &model) {
            const std::string data = C::encode(model);
            std::ofstream file(get_path(model.id), std::ios::binary | std::ios::trunc);
            file.write(data.data(), static_cast<std::streamsize>(data.size()));
        }

        std::optional<M> load(const UUID &id) {
            const auto path = get_path(id);
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                return std::nullopt;
            }
            const std::streamsize size = file.tellg();
            if (size <= 0) {
                return std::nullopt;
            }
            file.seekg(0, std::ios::beg);
            std::string buffer(static_cast<std::size_t>(size), '\0');
            if (!file.read(buffer.data(), size)) {
                return std::nullopt;
            }
            return C::decode(buffer);
        }

        std::vector<M> all() {
            std::vector<M> results;
            if (!std::filesystem::exists(directory_)) {
                return results;
            }
            for (const auto &entry: std::filesystem::directory_iterator(directory_)) {
                if (entry.is_regular_file() && entry.path().extension() == ".bin") {
                    auto id_str = entry.path().stem().string();
                    if (auto model = load(codec::HexCodec<UUID>::decode(id_str).value())) {
                        results.push_back(std::move(*model));
                    }
                }
            }
            return results;
        }

        [[nodiscard]] bool remove(const UUID &id) const { return std::filesystem::remove(get_path(id)); }

    private:
        std::filesystem::path directory_;

        [[nodiscard]] std::filesystem::path get_path(const UUID &id) const {
            return directory_ / (codec::HexCodec<UUID>::encode(id) + ".bin");
        }
    };

    namespace persistence {

        template<template<typename, typename> typename P, Model M, template<typename> typename C, typename... Args>
            requires Model<M> && std::is_constructible_v<P<M, C<M>>, Args...> && codec::Codec<C<M>, M> &&
                     Persistence<P<M, C<M>>, M>
        [[nodiscard]] auto create(Args &&...args) {
            return P<M, C<M>>(std::forward<Args>(args)...);
        }

    } // namespace persistence

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_PERSISTENCE_PERSISTENCE_H_
