//
// Created by Aman Mehara on 03/02/26.
//

#ifndef PRAPANCHA_MODEL_H
#define PRAPANCHA_MODEL_H

#include <concepts>
#include <string>

#include "uuid.h"

namespace mehara::prapancha {

    template<typename T>
    concept Model = requires(T m) {
        { T::ModelName } -> std::convertible_to<std::string_view>;
        { m.id } -> std::same_as<UUID &>;
    };

    struct BaseModel {
        UUID id{};
    };

    struct Author : BaseModel {
        static constexpr std::string_view ModelName = "author";

        std::string display_name;
        std::string bio;
    };

    struct Post : BaseModel {
        static constexpr std::string_view ModelName = "post";

        UUID author_id;
        std::string title;
        std::string content;
    };

    static_assert(Model<Author>, "Author does not satisfy the Model concept.");
    static_assert(Model<Post>, "Post does not satisfy the Model concept.");

} // namespace mehara::prapancha

#endif // PRAPANCHA_MODEL_H
