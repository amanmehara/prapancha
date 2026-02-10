//
// Created by Aman Mehara on 10/02/26.
//

#ifndef PRAPANCHA_CODEC_H
#define PRAPANCHA_CODEC_H

#include <concepts>
#include <iosfwd>

namespace mehara::prapancha {

    template<typename C, typename M>
    concept Codec = requires(std::ostream &os, std::istream &is, M &model) {
        { C::encode(os, model) } -> std::same_as<void>;
        { C::decode(is, model) } -> std::same_as<void>;
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CODEC_H
