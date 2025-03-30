#ifndef KEYHASH_HPP
#define KEYHASH_HPP

#include <bitset>
#include <cstddef>

namespace lea {

const size_t PRIME1 = 17;
const size_t PRIME2 = 31;

const size_t EXPAND_COMPACT_ITERATIONS = 4;

// constexpr unsigned char TWO_POW_256_MINUS_189 [32] = {
//     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x42};

struct keyhash {
    std::bitset<256> bits;
};

keyhash gen_keyhash(std::bitset<256>& input_bits,
                    size_t            input_byte_length);

// Bit-Interleaving Expansion
std::bitset<512> bit_interleaving_expand(std::bitset<256>& input_bits,
                                         size_t input_byte_length);

// unsigned long long pow(int base, int exp);
// Sequential Bit Compaction
std::bitset<256> sequential_bit_compact(std::bitset<512>& input_bits);
}    // namespace lea

#endif