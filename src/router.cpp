//
// Created by Aman Mehara on 01/02/26.
//

#include "router.h"

#include <filesystem>

#include "codec.h"
#include "controller/controller.h"
#include "model.h"
#include "persistence.h"

namespace mehara::prapancha {

    Router::Router(drogon::HttpAppFramework &app, const configuration::Configuration *configuration) :
        app_(app), configuration_(configuration) {
        register_routes();
    }

    template<typename T>
    auto Router::to_handler(const std::shared_ptr<T> &controller) {
        static_assert(std::is_base_of_v<BaseController<T>, T>, "The provided class must inherit from BaseController.");
        return [controller](const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            controller->dispatch(request, std::move(callback));
        };
    }

    void Router::register_routes() {
        const auto rootController = std::make_shared<RootController>();
        app_.registerHandler("/", to_handler(rootController), {drogon::Get});
        const std::string root_path = std::filesystem::absolute(configuration_->persistence.root_path).string();
        auto author_path = std::filesystem::absolute(root_path + "/" + std::string(Author::ModelName));
        auto post_path = std::filesystem::absolute(root_path + "/" + std::string(Post::ModelName));
        using AuthorPersistence = FilePersistence<Author, JsonCodec<Author>>;
        using AuthorController = AuthorController<AuthorPersistence, JsonCodec>;
        AuthorPersistence author_persistence = persistence::create<FilePersistence, Author, JsonCodec>(author_path);
        const auto authorController = std::make_shared<AuthorController>(std::move(author_persistence));
        app_.registerHandler("/authors", to_handler(authorController), {drogon::Get, drogon::Post});
        app_.registerHandler("/authors?id={id}", to_handler(authorController),
                             {drogon::Get, drogon::Put, drogon::Delete});
        using PostPersistence = FilePersistence<Post, JsonCodec<Post>>;
        using PostController = PostController<PostPersistence, JsonCodec>;
        PostPersistence post_persistence = persistence::create<FilePersistence, Post, JsonCodec>(post_path);
        const auto postController = std::make_shared<PostController>(std::move(post_persistence));
        app_.registerHandler("/posts", to_handler(postController), {drogon::Get, drogon::Post});
        app_.registerHandler("/posts?id={id}", to_handler(postController), {drogon::Get, drogon::Put, drogon::Delete});
    }

} // namespace mehara::prapancha
