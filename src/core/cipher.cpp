#include "cipher.hpp"

#include <array>
#include <cstdint>

namespace lea {

struct operation {
    uint8_t row, rowOffset, col, colOffset;
};

std::vector<operation> get_operations(const std::bitset<256>& key) {
    std::vector<operation> operations(16);

    for (uint8_t i = 0; i < 16; i++) {
        uint8_t start_index = 255 - (i * 16);

        uint16_t chunk = 0;
        for (size_t j = 0; j < 16; ++j) {
            chunk |= static_cast<uint16_t>(key[start_index - j]) << (15 - j);
        }

        operations[i].row       = (chunk >> 12) & 0xF;
        operations[i].rowOffset = (chunk >> 8) & 0xF;
        operations[i].col       = (chunk >> 4) & 0xF;
        operations[i].colOffset = (chunk) & 0xF;
    }

    return operations;
}

std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data,
                             const keyhash&              key) {
    std::vector<uint8_t> encrypted_data;

    return encrypted_data;
}

std::array<uint8_t, 32> cipher_block(const std::array<uint8_t, 32>& block,
                                     const std::vector<operation> operations) {
    // create bitset grid
    std::array<std::bitset<16>, 16> grid;
    for (uint16_t i = 0; i < block.size(); i += 2) {
        uint16_t combined
            = (static_cast<uint16_t>(block[i + 1]) << 8) | block[i];

        grid[i / 2] = std::bitset<16>(combined);
    }

    // ciphering
    for (const operation& op : operations) {
        std::bitset<16> row = grid[op.row];
        grid[op.row] = (row << op.rowOffset) | (row >> (16 - op.rowOffset));

        std::bitset<16> col;
        for (uint8_t r = 0; r < 16; ++r) { col[r] = grid[r][op.col]; }

        std::bitset<16> shifted_col
            = (col << op.colOffset) | (col >> (16 - op.colOffset));
        for (uint8_t r = 0; r < 16; ++r) { grid[r][op.col] = shifted_col[r]; }
    }

    // convert back to bytes
    std::array<uint8_t, 32> result;
    for (uint8_t i = 0; i < 16; ++i) {
        uint16_t val      = static_cast<uint16_t>(grid[i].to_ulong());
        result[i * 2]     = val & 0xFF;
        result[i * 2 + 1] = (val >> 8) & 0xFF;
    }
    return result;
}

std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data,
                             const keyhash&              key) { }

}  // namespace lea