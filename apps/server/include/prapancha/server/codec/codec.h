//
// Created by Aman Mehara on 10/02/26.
//

#ifndef PRAPANCHA_SERVER_CODEC_CODEC_H_
#define PRAPANCHA_SERVER_CODEC_CODEC_H_

#include <concepts>
#include <optional>

namespace mehara::prapancha::codec {

    template<typename C, typename T>
    concept Codec = requires(const T &value, const typename C::encoded_view &data) {
        typename C::encoded_type;
        typename C::encoded_view;
        requires std::convertible_to<typename C::encoded_type, typename C::encoded_view>;
        { C::encode(value) } -> std::same_as<typename C::encoded_type>;
        { C::decode(data) } -> std::same_as<std::optional<T>>;
    };

} // namespace mehara::prapancha::codec

#endif // PRAPANCHA_SERVER_CODEC_CODEC_H_
