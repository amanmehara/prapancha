//
// Created by Aman Mehara on 10/02/26.
//

#ifndef PRAPANCHA_CODEC_H
#define PRAPANCHA_CODEC_H

#include <concepts>

#include "model.h"

namespace mehara::prapancha {

    template<typename C, typename M>
    concept Codec = requires(const M &model, const typename C::EncodedType &data) {
        requires Model<M>;
        typename C::EncodedType;
        { C::encode(model) } -> std::same_as<typename C::EncodedType>;
        { C::decode(data) } -> std::same_as<std::optional<M>>;
    };

    template<Model M>
    struct BinaryCodec {
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CODEC_H
