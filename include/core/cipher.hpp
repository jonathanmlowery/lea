#ifndef CIPHER_HPP
#define CIPHER_HPP

#include <cstdint>
#include <vector>

#include "keyhash.hpp"

enum Mode {
    DECRYPT = -1,
    UNSET   = 0,
    ENCRYPT = 1,
};

namespace lea {

std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data,
                             const keyhash&              key);

std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data,
                             const keyhash&              key);
}    // namespace lea

#endif