#include "keyhash.hpp"

#include <gtest/gtest.h>

#include <bitset>

TEST(KeyhashTest, AvalancheEffect) {
    // Test case 1: "abc" vs "abd"
    std::string      s1     = "abc";
    std::string      s2     = "abd";
    std::bitset<256> input1 = lea::bitify_str(s1);
    std::bitset<256> input2 = lea::bitify_str(s2);

    lea::keyhash hash1 = lea::gen_keyhash(input1, s1.size());
    lea::keyhash hash2 = lea::gen_keyhash(input2, s2.size());

    std::bitset<256> diff  = hash1.bits ^ hash2.bits;
    size_t           flips = diff.count();
    EXPECT_GE(flips, 100) << "Avalanche effect weak: " << flips
                          << " flips for 'abc' vs 'abd'";

    // Test case 2: "hello world" vs "hello world!"
    s1     = "hello world";
    s2     = "hello world!";
    input1 = lea::bitify_str(s1);
    input2 = lea::bitify_str(s2);

    hash1 = lea::gen_keyhash(input1, s1.size());
    hash2 = lea::gen_keyhash(input2, s2.size());

    diff  = hash1.bits ^ hash2.bits;
    flips = diff.count();
    EXPECT_GE(flips, 100) << "Avalanche effect weak: " << flips
                          << " flips for 'hello world' vs 'hello world!'";

    // Test case 3: int 254 vs int 255
    input1 = std::bitset<256>(254);
    input2 = std::bitset<256>(255);

    hash1 = lea::gen_keyhash(input1, s1.size());
    hash2 = lea::gen_keyhash(input2, s2.size());

    diff  = hash1.bits ^ hash2.bits;
    flips = diff.count();
    EXPECT_GE(flips, 100) << "Avalanche effect weak: " << flips
                          << " flips for 254 vs 255";
}

TEST(KeyhashTest, NoLongSequences) {
    std::string      s     = "hello world";
    std::bitset<256> input = lea::bitify_str(s);
    lea::keyhash     hash  = lea::gen_keyhash(input, s.size());
    std::bitset<256> bits  = hash.bits;

    size_t max_run_0 = 0, max_run_1 = 0;
    size_t current_run_0 = 0, current_run_1 = 0;

    for (size_t i = 0; i < 256; i++) {
        if (bits [i]) {
            current_run_1++;
            current_run_0 = 0;
            max_run_1     = std::max(max_run_1, current_run_1);
        } else {
            current_run_0++;
            current_run_1 = 0;
            max_run_0     = std::max(max_run_0, current_run_0);
        }
    }

    EXPECT_LE(max_run_0, 20) << "Too many consecutive 0s: " << max_run_0;
    EXPECT_LE(max_run_1, 20) << "Too many consecutive 1s: " << max_run_1;
}

TEST(KeyhashTest, EmptyInput) {
    std::bitset<256> input;
    lea::keyhash     hash = lea::gen_keyhash(input, 1);

    EXPECT_NE(hash.bits.count(), 0) << "Empty input hashes to all zeros";
    EXPECT_NE(hash.bits.count(), 256) << "Empty input hashes to all ones";
}

TEST(KeyhashTest, CollisionResistance) {
    std::string      s1     = "foobar";
    std::string      s2     = "barfoo";
    std::bitset<256> input1 = lea::bitify_str(s1);
    std::bitset<256> input2 = lea::bitify_str(s2);

    lea::keyhash hash1 = lea::gen_keyhash(input1, s1.size());
    lea::keyhash hash2 = lea::gen_keyhash(input2, s2.size());

    EXPECT_NE(hash1.bits, hash2.bits)
        << "Collision detected: 'foobar' and 'barfoo' hash to same value";

    size_t flips = (hash1.bits ^ hash2.bits).count();
    EXPECT_GE(flips, 50) << "Weak difference: " << flips
                         << " flips for 'foobar' vs 'barfoo'";
}