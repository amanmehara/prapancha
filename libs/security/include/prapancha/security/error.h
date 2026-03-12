//
// Created by Aman Mehara on 10/03/26.
//

#ifndef PRAPANCHA_SECURITY_ERROR_H_
#define PRAPANCHA_SECURITY_ERROR_H_

#include <openssl/bio.h>
#include <openssl/err.h>

#include <memory>
#include <optional>
#include <string>

namespace mehara::prapancha::security {

    class Error final {
    public:
        explicit constexpr Error(const unsigned long code) noexcept : code_(code) {}

        [[nodiscard]] constexpr unsigned long code() const noexcept {
            return code_;
        }

        [[nodiscard]] std::optional<std::string> description() const {
            if (code_ == 0) {
                return std::nullopt;
            }
            const std::unique_ptr<BIO, decltype(&BIO_free)> bio(BIO_new(BIO_s_mem()), BIO_free);
            if (!bio) {
                return std::nullopt;
            }
            ERR_print_errors(bio.get());
            char *data = nullptr;
            const long len = BIO_get_mem_data(bio.get(), &data);
            if (len <= 0 || !data) {
                return std::nullopt;
            }
            return std::string(data, static_cast<std::size_t>(len));
        }

    private:
        unsigned long code_;
    };

} // namespace mehara::prapancha::security

#endif // PRAPANCHA_SECURITY_ERROR_H_
