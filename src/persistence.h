//
// Created by Aman Mehara on 03/02/26.
//

#ifndef PRAPANCHA_PERSISTENCE_H
#define PRAPANCHA_PERSISTENCE_H

#include <filesystem>
#include <fstream>

#include "codec.h"
#include "model.h"
#include "uuid.h"

namespace mehara::prapancha {

    template<typename P, typename M>
    concept PersistencePolicy = requires(P policy, const M &model, const UUID &id) {
        requires Model<M>;
        { policy.save(model) } -> std::same_as<void>;
        { policy.load(id) } -> std::same_as<std::optional<M>>;
        { policy.all() } -> std::same_as<std::vector<M>>;
        { policy.remove(id) } -> std::same_as<bool>;
    };

    template<typename M, typename C>
        requires Codec<C, M> && std::same_as<typename C::EncodedType, std::string>
    class FilePersistencePolicy {
    public:
        explicit FilePersistencePolicy(std::filesystem::path path) : _dir(std::move(path)) {
            if (!std::filesystem::exists(_dir)) {
                std::filesystem::create_directories(_dir);
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
            if (!std::filesystem::exists(_dir)) {
                return results;
            }
            for (const auto &entry: std::filesystem::directory_iterator(_dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".bin") {
                    auto id_str = entry.path().stem().string();
                    if (auto model = load(UUID::from_hex(id_str).value())) {
                        results.push_back(std::move(*model));
                    }
                }
            }
            return results;
        }

        [[nodiscard]] bool remove(const UUID &id) const { return std::filesystem::remove(get_path(id)); }

    private:
        std::filesystem::path _dir;

        [[nodiscard]] std::filesystem::path get_path(const UUID &id) const { return _dir / (id.to_hex() + ".bin"); }
    };

    class PersistenceFactory {
    public:
        template<Model M, Codec<M> C>
        static auto create_persistence(const std::string_view root_path) {
            return FilePersistencePolicy<M, C>(std::filesystem::path(root_path) / M::ModelName);
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_PERSISTENCE_H
