#include "keyhash.hpp"

#include <bitset>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace lea {

std::string keyhash::hex_str() const noexcept {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (int i = 3; i >= 0; --i) {
        uint64_t chunk = 0;
        for (int j = 0; j < 64; ++j) {
            chunk |= static_cast<uint64_t>(bits [i * 64 + j]) << (63 - j);
        }
        oss << std::setw(16) << chunk;
    }
    return oss.str();
}

keyhash gen_keyhash(const std::bitset<256>& input_bits,
                    size_t                  input_byte_length) {
    std::bitset<512> expanded_bits = bit_interleaving_expand(input_bits,
                                                             input_byte_length);

    std::bitset<256> compacted_bits = sequential_bit_compact(expanded_bits);

    compacted_bits = rotate_left(compacted_bits,
                                 (compacted_bits.count() * PRIMES [0]) % 256);

    mix(compacted_bits, 1);
    apply_sbox(compacted_bits);
    intermittent_bit_flip(compacted_bits);

    for (size_t i = 1; i < EXPAND_COMPACT_ITERATIONS; i++) {
        expanded_bits = bit_interleaving_expand(compacted_bits, 32);

        compacted_bits = sequential_bit_compact(expanded_bits);
        compacted_bits = rotate_left(
            compacted_bits,
            (compacted_bits.count() * PRIMES [i]) % 256);

        mix(compacted_bits, i + 1);
        apply_sbox(compacted_bits);
        intermittent_bit_flip(compacted_bits);
    }

    return keyhash {compacted_bits};
}

std::bitset<512> bit_interleaving_expand(const std::bitset<256>& input_bits,
                                         size_t input_byte_length) {

    std::bitset<256> padding_bits;
    std::bitset<256> wrapping_input_bits;

    std::bitset<512> expanded_input;

    for (size_t byte_index = 0; byte_index < 32; byte_index++) {
        size_t wrapped_byte_index = byte_index % input_byte_length;

        std::bitset<8> byte;
        for (size_t bit_index = 0; bit_index < 8; bit_index++) {
            byte [bit_index] = input_bits [8 * wrapped_byte_index + bit_index];
        }

        uint8_t byte_val        = static_cast<uint8_t>(byte.to_ulong());
        uint8_t transformed_val = static_cast<uint8_t>(byte_val * PRIME1);
        std::bitset<8> transformed_byte(transformed_val);
        transformed_byte.flip();

        for (size_t bit_index = 0; bit_index < 8; bit_index++) {
            wrapping_input_bits [byte_index * 8 + bit_index] = byte [bit_index];
            padding_bits [byte_index * 8 + bit_index]        = transformed_byte
                [bit_index];
        }
    }

    for (size_t bit_index = 0; bit_index < 512; bit_index++) {
        if (bit_index % 2) {
            expanded_input [bit_index] = padding_bits [bit_index / 2];
        } else {
            expanded_input [bit_index] = wrapping_input_bits [255
                                                              - (bit_index
                                                                 / 2)];
        }
    }

    return expanded_input;
}

std::bitset<256> sequential_bit_compact(const std::bitset<512>& input_bits) {
    std::bitset<256> compacted_input;

    for (int i = 0; i < 512; i += 2) {
        compacted_input [i / 2] = input_bits [i] ^ input_bits [i + 1];
    }

    return compacted_input;
}

std::bitset<256> rotate_left(const std::bitset<256>& bits, size_t shift) {
    return (bits << shift) | (bits >> (256 - shift));
}

std::bitset<256> bitify_str(const std::string& str) {
    std::bitset<256> bits;

    for (size_t i = 0; i < str.size() && i * 8 < 256; i++) {
        unsigned char byte = static_cast<unsigned char>(str [i]);
        for (size_t j = 0; j < 8; j++) { bits [i * 8 + j] = (byte >> j) & 1; }
    }

    return bits;
}

void intermittent_bit_flip(std::bitset<256>& bits) {
    for (size_t i = 0; i < 256; i += PRIME2) { bits [i] = bits [i] ^ 1; }
}

void apply_sbox(std::bitset<256>& bits) {
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&bits);
    for (size_t j = 0; j < 32; j++) {
        uint32_t x      = bytes [j];
        uint32_t result = (x * PRIME1) ^ (x * x);

        // Fold the 32 bits result (4 bytes) back down to 8 bits (1 byte)
        uint8_t compacted = (result & 0xFF) ^ ((result >> 8) & 0xFF)
                          ^ ((result >> 16) & 0xFF) ^ ((result >> 24) & 0xFF);

        bytes [j] = compacted;
    }
}

void mix(std::bitset<256>& bits, size_t round) {
    for (size_t j = 0; j < 256; j++) {
        bits [j] = bits [j] ^ ((j + round * PRIME2) % 2);
    }
}

}    // namespace lea