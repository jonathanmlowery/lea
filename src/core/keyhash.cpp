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

    std::bitset<256> compacted_bits = sequential_bit_compact(
        expanded_bits);

    // xor_round_constant(compacted_bits, 1);
    intermittent_bit_flip(compacted_bits);
    // mix(compacted_bits, 1);
    compacted_bits ^= std::bitset<256>(input_byte_length);
    compacted_bits  = rotate_left(compacted_bits,
                                 compacted_bits.count() % PRIMES [0]);
    // apply_sbox(compacted_bits);

    for (size_t i = 1; i < EXPAND_COMPACT_ITERATIONS; i++) {
        expanded_bits = bit_interleaving_expand(compacted_bits, 32);

        compacted_bits = sequential_bit_compact(expanded_bits);
        // compacted_bits = rotate_left(compacted_bits, PRIMES [i]);
        compacted_bits = rotate_left(compacted_bits,
                                     compacted_bits.count() % PRIMES [i]);

        // compacted_bits = modulo_bitset(compacted_bits, 2);
        // xor_round_constant(compacted_bits, i);
        // mix(compacted_bits, i);
        intermittent_bit_flip(compacted_bits);
        compacted_bits ^= std::bitset<256>(input_byte_length);
        // apply_sbox(compacted_bits);
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

        std::bitset<8> byte;
        for (size_t bit_index = 0; bit_index < 8; bit_index++) {
            byte [bit_index] = input_bits [8 * wrapped_byte_index
                                           + bit_index];
        }

        uint8_t byte_val        = static_cast<uint8_t>(byte.to_ulong());
        uint8_t transformed_val = static_cast<uint8_t>(byte_val * PRIME1);
        std::bitset<8> transformed_byte(transformed_val);
        transformed_byte.flip();

        for (size_t bit_index = 0; bit_index < 8; bit_index++) {
            wrapping_input_bits [byte_index * 8 + bit_index] = byte
                [bit_index];
            padding_bits [byte_index * 8 + bit_index] = transformed_byte
                [bit_index];
        }
    }

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

std::bitset<256> rotate_left(std::bitset<256>& bits, size_t shift) {
    return (bits << shift) | (bits >> (256 - shift));
}

std::bitset<256> bitify_str(std::string str) {
    std::bitset<256> bits;

    for (size_t i = 0; i < str.size() && i * 8 < 256; i++) {
        unsigned char byte = static_cast<unsigned char>(str [i]);
        for (size_t j = 0; j < 8; j++) {
            bits [i * 8 + j] = (byte >> j) & 1;
        }
    }

    return bits;
}

void intermittent_bit_flip(std::bitset<256>& bits) {
    for (size_t i = 0; i < 256; i += PRIME2) { bits [i] = bits [i] ^ 1; }
}

void xor_round_constant(std::bitset<256>& bits, size_t round) {
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&bits);
    for (size_t j = 0; j < 32; j++) {
        bytes [j] ^= (j * PRIME1 + round * PRIME2) & 0xFF;
    }
}

void apply_sbox(std::bitset<256>& bits) {
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&bits);
    for (size_t j = 0; j < 32; j++) { bytes [j] ^= (bytes [j] * 17); }
}

void mix(std::bitset<256>& bits, size_t round) {
    for (size_t j = 0; j < 256; j++) {
        bits [j] = bits [j] ^ ((j + round * PRIME2) % 2);
    }
}

std::bitset<256> modulo_bitset(std::bitset<256>& bits, uint64_t modulus) {
    // divide into 4 chunks of 64b
    uint64_t chunks [4] = {0};
    for (size_t i = 0; i < 256; i++) {
        if (bits [i]) { chunks [i / 64] |= (1ULL << (i % 64)); }
    }

    uint64_t result = chunks [0] % modulus;

    std::bitset<256> modded_bits;
    for (size_t i = 0; i < 64 && i < 256; i++) {
        modded_bits [i] = (result >> i) & 1;
    }
    return modded_bits;
}

}    // namespace lea