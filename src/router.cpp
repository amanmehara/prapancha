//
// Created by Aman Mehara on 01/02/26.
//

#include "router.h"

#include "codec.h"
#include "controller.h"
#include "model.h"
#include "persistence.h"

namespace mehara::prapancha {

    template<typename T>
    auto Router::toHandler(const std::shared_ptr<T> &controller) {
        static_assert(std::is_base_of_v<BaseController<T>, T>,
                      "The provided class must inherit from BaseController<T>.");
        return [controller](const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            controller->dispatch(request, std::move(callback));
        };
    }

    void Router::configure(drogon::HttpAppFramework &app) {
        const auto rootController = std::make_shared<RootController>();
        app.registerHandler("/", toHandler(rootController), {drogon::Get});

        const std::string rootPath = std::filesystem::absolute("./persistence").string();

        using AuthorPersistence = FilePersistence<Author, JsonCodec<Author>>;
        using AuthorController = AuthorController<AuthorPersistence, JsonCodec>;
        AuthorPersistence authorPersistence =
                PersistenceFactory::create_persistence<Author, JsonCodec<Author>>(rootPath);
        const auto authorController = std::make_shared<AuthorController>(std::move(authorPersistence));

        app.registerHandler("/authors", toHandler(authorController), {drogon::Get, drogon::Post});
        app.registerHandler("/authors?id={id}", toHandler(authorController),
                            {drogon::Get, drogon::Put, drogon::Delete});

        using PostPersistence = FilePersistence<Post, JsonCodec<Post>>;
        using PostController = PostController<PostPersistence, JsonCodec>;
        PostPersistence postPersistence = PersistenceFactory::create_persistence<Post, JsonCodec<Post>>(rootPath);
        const auto postController = std::make_shared<PostController>(std::move(postPersistence));

        app.registerHandler("/posts", toHandler(postController), {drogon::Get, drogon::Post});
        app.registerHandler("/posts?id={id}", toHandler(postController), {drogon::Get, drogon::Put, drogon::Delete});
    }

} // namespace mehara::prapancha
