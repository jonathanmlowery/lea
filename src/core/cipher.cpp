#include "cipher.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

namespace lea {

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

    // pad data to a multiple of 32 bytes
    std::vector<uint8_t> padded_data = data;
    size_t               pad_len     = 32 - (padded_data.size() % 32);
    if (pad_len != 32) {
        padded_data.insert(padded_data.end(),
                           pad_len,
                           static_cast<uint8_t>(pad_len));
    }

    auto operations = get_operations(key.bits);

    // encrypt each
    for (size_t i = 0; i < padded_data.size(); i += 32) {
        std::array<uint8_t, 32> block{};
        std::copy_n(padded_data.begin() + i, 32, block.begin());
        auto encrypted_block = cipher_block(block, operations);
        encrypted_data.insert(encrypted_data.end(),
                              encrypted_block.begin(),
                              encrypted_block.end());
    }

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
                             const keyhash&              key) {
    std::vector<uint8_t> decrypted_data;

    // invalid ciphertext size
    if (data.size() % 32 != 0) { return {}; }

    auto                   operations   = get_operations(key.bits);
    std::vector<operation> reversed_ops = operations;
    std::reverse(reversed_ops.begin(), reversed_ops.end());

    for (size_t i = 0; i < data.size(); i += 32) {
        std::array<uint8_t, 32> block{};
        std::copy_n(data.begin() + i, 32, block.begin());
        auto decrypted_block = decipher_block(block, reversed_ops);
        decrypted_data.insert(decrypted_data.end(),
                              decrypted_block.begin(),
                              decrypted_block.end());
    }

    // remove extra padding if present
    if (!decrypted_data.empty()) {
        uint8_t pad_len = decrypted_data.back();
        if (pad_len > 0 && pad_len <= 32
            && std::all_of(decrypted_data.end() - pad_len,
                           decrypted_data.end(),
                           [pad_len](uint8_t b) { return b == pad_len; })) {
            decrypted_data.resize(decrypted_data.size() - pad_len);
        }
    }

    return decrypted_data;
}

std::array<uint8_t, 32> decipher_block(
    const std::array<uint8_t, 32>& block,
    const std::vector<operation>   operations) {
    // create bitset grid
    std::array<std::bitset<16>, 16> grid;
    for (uint16_t i = 0; i < block.size(); i += 2) {
        uint16_t combined
            = (static_cast<uint16_t>(block[i + 1]) << 8) | block[i];
        grid[i / 2] = std::bitset<16>(combined);
    }

    // deciphering: for each op, undo col shift, then row shift
    for (const operation& op : operations) {
        // Undo column shift first
        std::bitset<16> col;
        for (uint8_t r = 0; r < 16; ++r) { col[r] = grid[r][op.col]; }
        std::bitset<16> shifted_col
            = (col >> op.colOffset) | (col << (16 - op.colOffset));
        for (uint8_t r = 0; r < 16; ++r) { grid[r][op.col] = shifted_col[r]; }

        // Then undo row shift
        std::bitset<16> row = grid[op.row];
        grid[op.row] = (row >> op.rowOffset) | (row << (16 - op.rowOffset));
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

}  // namespace lea