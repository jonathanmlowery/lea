#ifndef KEYHASH_HPP
#define KEYHASH_HPP

#include <bitset>
#include <cstddef>

namespace lea {

const size_t PRIME1 = 17;
const size_t PRIME2 = 31;

// constexpr unsigned char TWO_POW_256_MINUS_189 [32] = {
//     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x42};

// Bit-Interleaving Expansion
std::bitset<512> bit_interleaving_expand(std::bitset<256>& input,
                                         size_t            length);

// unsigned long long pow(int base, int exp);
// Sequential Bit Compaction
std::bitset<256> sequential_bit_compact(std::bitset<512>& input);
}    // namespace lea

#endif