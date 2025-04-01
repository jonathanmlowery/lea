#include "keyhash.hpp"

#include <gtest/gtest.h>

#include <bitset>
#include <random>
#include <ratio>
#include <unordered_set>
#include <vector>

// Helper to generate a random 256-bit bitset
std::random_device rd;
std::mt19937_64    gen(rd());

std::bitset<256> generate_random_bitset() {
    std::bitset<256> bits;

    for (size_t i = 0; i < 256; i += 64) {
        uint64_t chunk = gen() & 0xFF'FF'FF'FF'FF'FF'FF'FFULL;
        for (size_t j = 0; j < 64 && (i + j) < 256; j++) {
            bits [i + j] = (chunk >> j) & 1;
        }
    }
    return bits;
}

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

    // Test case 4: "abc" vs "abcd" as a full 256 bit input
    s1     = "abc";
    s2     = "abcd";
    input1 = lea::bitify_str(s1);
    input2 = lea::bitify_str(s2);

    hash1 = lea::gen_keyhash(input1, 32);
    hash2 = lea::gen_keyhash(input2, 32);

    diff  = hash1.bits ^ hash2.bits;
    flips = diff.count();
    EXPECT_GE(flips, 100) << "Avalanche effect weak: " << flips
                          << " flips for 'abc' vs 'abcd'";
}

TEST(KeyhashTest, FullInputLateBitAvalanche) {
    std::bitset<256> input1;
    input1.set(255);
    std::bitset<256> input2 = input1;
    input2.flip(255);
    lea::keyhash hash1 = lea::gen_keyhash(input1, 32);
    lea::keyhash hash2 = lea::gen_keyhash(input2, 32);
    size_t       flips = (hash1.bits ^ hash2.bits).count();
    EXPECT_GE(flips, 100) << "Flips: " << flips;
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
    lea::keyhash     hash = lea::gen_keyhash(input, 256);

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

TEST(KeyhashTest, CollisionResistance_LargeRandomInputs) {
    const size_t                  NUM_INPUTS = 100'000;    // use at least 1M
    std::vector<std::bitset<256>> inputs(NUM_INPUTS);
    std::unordered_set<std::bitset<256>> hash_outputs;

    for (auto& input : inputs) { input = generate_random_bitset(); }

    auto time_start      = std::chrono::high_resolution_clock::now();
    auto time_last_print = time_start;

    for (size_t i = 0; i < NUM_INPUTS; i++) {
        lea::keyhash hash = lea::gen_keyhash(inputs [i], 32);

        auto [it, inserted] = hash_outputs.insert(hash.bits);

        EXPECT_TRUE(inserted)
            << "Collision detected at input " << i << ": hash = " << hash.bits;

        auto time_current = std::chrono::high_resolution_clock::now();

        double delta_time_ms = std::chrono::duration<double, std::milli>(
                                   time_current - time_last_print)
                                   .count();
        // Optional: Progress log for big runs
        if (delta_time_ms > 500) {
            std::cout << "Processed " << i << " inputs..." << std::endl;
            time_last_print = time_current;
        }
    }

    auto time_end = std::chrono::high_resolution_clock::now();

    double time_us = std::chrono::duration<double, std::micro>(
                         time_end - time_start)
                         .count();
    double avg_time_us = time_us / NUM_INPUTS;

    std::cout << "Hashed " << NUM_INPUTS << " inputs in " << time_us / 1'000
              << " ms\n";
    std::cout << "Average time per hash: " << avg_time_us << " µs\n";

    EXPECT_EQ(hash_outputs.size(), NUM_INPUTS)
        << "Expected " << NUM_INPUTS << " unique hashes, got "
        << hash_outputs.size();
}

TEST(KeyhashTest, HashTiming) {
    const size_t       NUM_INPUTS = 1'000;
    std::random_device rd;
    std::mt19937_64    gen(rd());

    // Generate random inputs beforehand to exclude generation time from
    // hash timing
    std::vector<std::bitset<256>> inputs(NUM_INPUTS);
    for (auto& input : inputs) { input = generate_random_bitset(); }

    // Measure only the hashing time
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& input : inputs) {
        lea::keyhash hash = lea::gen_keyhash(input, 32);
    }
    auto end = std::chrono::high_resolution_clock::now();

    double time_us = std::chrono::duration<double, std::micro>(end - start)
                         .count();
    double avg_time_us = time_us / NUM_INPUTS;

    std::cout << "Hashed " << NUM_INPUTS << " inputs in " << time_us / 1'000
              << " ms\n";
    std::cout << "Average time per hash: " << avg_time_us << " µs\n";

    SUCCEED();
}