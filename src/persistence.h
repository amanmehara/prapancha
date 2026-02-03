//
// Created by Aman Mehara on 03/02/26.
//

#ifndef PRAPANCHA_PERSISTENCE_H
#define PRAPANCHA_PERSISTENCE_H

#include <filesystem>
#include <fstream>
#include <sstream>

#include "model.h"

namespace mehara::prapancha {

    std::string uuid_to_hex(const uuid_t &id);

    template<typename P, typename T>
    concept PersistencePolicy = requires(P policy, const T &model, const uuid_t &id) {
        { policy.save(model) } -> std::same_as<void>;
        { policy.load(id) } -> std::same_as<std::optional<T>>;
        { policy.all() } -> std::same_as<std::vector<T>>;
        { policy.remove(id) } -> std::same_as<bool>;
    };

    template<Model T>
    class FilePersistencePolicy {
    public:
        explicit FilePersistencePolicy(std::filesystem::path path) : _dir(std::move(path)) {
            if (!std::filesystem::exists(_dir)) {
                std::filesystem::create_directories(_dir);
            }
        }

        void save(const T &model) {
            std::ofstream file(get_path(model.id), std::ios::binary | std::ios::trunc);
            file.write(reinterpret_cast<const char *>(&model), sizeof(T));
        }

        std::optional<T> load(const uuid_t &id) {
            auto path = get_path(id);
            if (!std::filesystem::exists(path)) {
                return std::nullopt;
            }
            T model;
            std::ifstream file(path, std::ios::binary);
            file.read(reinterpret_cast<char *>(&model), sizeof(T));
            return model;
        }

        std::vector<T> all() {
            std::vector<T> results;
            if (!std::filesystem::exists(_dir)) {
                return results;
            }
            for (const auto &entry: std::filesystem::directory_iterator(_dir)) {
                if (entry.path().extension() == ".bin") {
                    T model;
                    if (std::ifstream file(entry.path(), std::ios::binary);
                        file.read(reinterpret_cast<char *>(&model), sizeof(T))) {
                        results.push_back(std::move(model));
                    }
                }
            }
            return results;
        }

        [[nodiscard]] bool remove(const uuid_t &id) const { return std::filesystem::remove(get_path(id)); }

    private:
        std::filesystem::path _dir;

        [[nodiscard]] std::filesystem::path get_path(const uuid_t &id) const {
            return _dir / (uuid_to_hex(id) + ".bin");
        }
    };

    class PersistenceFactory {
    public:
        template<Model T>
        static auto create_persistence(std::string_view root_path) {
            return FilePersistencePolicy<T>(std::filesystem::path(root_path) / T::ModelName);
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_PERSISTENCE_H
