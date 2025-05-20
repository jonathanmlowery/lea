#ifndef CIPHER_HPP
#define CIPHER_HPP

#include <array>
#include <cstdint>
#include <vector>

#include "keyhash.hpp"

enum Mode {
    DECRYPT = -1,
    UNSET   = 0,
    ENCRYPT = 1,
};

struct operation {
    uint8_t row, rowOffset, col, colOffset;
};

namespace lea {

std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data,
                             const keyhash&              key);

std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data,
                             const keyhash&              key);

std::array<uint8_t, 32> cipher_block(const std::array<uint8_t, 32>& block,
                                     const std::vector<operation>   operations);

std::array<uint8_t, 32> decipher_block(const std::array<uint8_t, 32>& block,
                                       const std::vector<operation> operations);

std::vector<operation> get_operations(const std::bitset<256>& key);

}  // namespace lea

#endif