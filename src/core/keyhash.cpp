#include "keyhash.hpp"

#include <bitset>
#include <cmath>
#include <iostream>
#include <utility>

namespace lea {

keyhash gen_keyhash(std::bitset<256>& input_bits,
                    size_t            input_byte_length) {
    std::bitset<512> expanded_bits = bit_interleaving_expand(
        input_bits,
        input_byte_length);

    std::cout << "Expanded: \n" << expanded_bits << '\n';

    std::bitset<256> compacted_bits = sequential_bit_compact(
        expanded_bits);

    std::cout << "Compacted: \n" << compacted_bits << '\n';

    for (size_t i = 1; i < EXPAND_COMPACT_ITERATIONS; i++) {
        expanded_bits = bit_interleaving_expand(compacted_bits, 32);
        std::cout << "Expanded: \n" << expanded_bits << '\n';

        compacted_bits = sequential_bit_compact(expanded_bits);
        std::cout << "Compacted: \n" << compacted_bits << '\n';
    }

    return keyhash {compacted_bits};
}

std::bitset<512> bit_interleaving_expand(std::bitset<256>& input_bits,
                                         size_t input_byte_length) {

    std::bitset<256> padding_bits;
    std::bitset<256> wrapping_input_bits;

    std::bitset<512> expanded_input;

    for (size_t byte_index = 0; byte_index < 32; byte_index++) {
        size_t wrapped_byte_index = byte_index % input_byte_length;
        // std::cout << wrapped_byte_index << " : ";
        std::bitset<8> byte;
        for (size_t bit_index = 0; bit_index < 8; bit_index++) {
            byte [bit_index] = input_bits [8 * wrapped_byte_index
                                           + bit_index];
        }

        uint8_t byte_val        = static_cast<uint8_t>(byte.to_ulong());
        uint8_t transformed_val = static_cast<uint8_t>(byte_val * PRIME1);
        std::bitset<8> transformed_byte(transformed_val);

        // std::cout << "Original: " << byte << '\n';
        // std::cout << "Transformed: " << transformed_byte << '\n';

        for (size_t bit_index = 0; bit_index < 8; bit_index++) {
            wrapping_input_bits [byte_index * 8 + bit_index] = byte
                [bit_index];
            padding_bits [byte_index * 8 + bit_index] = transformed_byte
                [bit_index];
        }
    }

    // std::cout << padding_bits << '\n';
    // std::cout << wrapping_input_bits << '\n';

    for (size_t bit_index = 0; bit_index < 512; bit_index++) {
        if (bit_index % 2) {
            expanded_input [bit_index] = padding_bits [bit_index / 2];
        } else {
            expanded_input [bit_index] = wrapping_input_bits [256
                                                              - (bit_index
                                                                 / 2)];
        }
    }

    return expanded_input;
}

std::bitset<256> sequential_bit_compact(std::bitset<512>& input_bits) {
    std::bitset<256> compacted_input;

    for (int i = 0; i < 512; i += 2) {
        compacted_input [i / 2] = input_bits [i] ^ input_bits [i + 1];
    }

    return compacted_input;
}

}    // namespace lea