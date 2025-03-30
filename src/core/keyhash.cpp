#include "keyhash.hpp"

#include <cmath>
#include <iostream>

namespace lea {

std::bitset<512> bit_interleaving_expand(std::bitset<256>& input,
                                         size_t            length) {

    std::bitset<256> bit_padding;
    std::bitset<512> expanded_input;
    unsigned char*   I = (unsigned char*)&input;

    for (size_t byte_index = 0; byte_index < 32; byte_index++) {
        char byte1 = I [byte_index % (length / 8)] * PRIME1;
        std::cout << "Byte: " << (std::bitset<8>)byte1 << '\n';

        for (size_t bit_index = 0; bit_index < 8; bit_index++) {
            bit_padding [byte_index * 8 + bit_index] = (byte1
                                                        >> (bit_index))
                                                     & 1;
        }
    }

    for (size_t bit_index = 0; bit_index < 512; bit_index++) {
        if (bit_index % 2) {
            expanded_input [bit_index] = bit_padding [bit_index / 2];
        } else {
            expanded_input [bit_index] = input [(bit_index / 2) % length];
        }
    }

    return expanded_input;
}

std::bitset<256> sequential_bit_compact(std::bitset<512>& input) {
    std::bitset<256> compacted_input;

    for (int i = 0; i < 512; i += 2) {
        compacted_input [i / 2] = input [i] ^ input [i + 1];
    }

    return compacted_input;
}

}    // namespace lea