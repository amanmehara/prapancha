//
// Created by Aman Mehara on 03/02/26.
//

#include "persistence.h"

namespace mehara::prapancha {

    std::string uuid_to_hex(const uuid_t &id) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (auto byte: id) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }

} // namespace mehara::prapancha
