#include "cipher.hpp"

#include <gtest/gtest.h>

#include <bitset>
#include <random>

#include "cipher.hpp"

using namespace lea;

keyhash make_key(uint64_t seed = 0) {
    std::bitset<256> bits;
    std::mt19937_64  rng(seed);
    for (size_t i = 0; i < 256; ++i) { bits[i] = rng() & 1; }
    return keyhash{bits};
}

TEST(CipherTest, EncryptDecryptBasic) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    keyhash              key  = make_key(42);

    auto encrypted = encrypt(data, key);
    auto decrypted = decrypt(encrypted, key);

    EXPECT_EQ(decrypted, data);
}

TEST(CipherTest, EncryptDecryptEmpty) {
    std::vector<uint8_t> data;
    keyhash              key = make_key(123);

    auto encrypted = encrypt(data, key);
    auto decrypted = decrypt(encrypted, key);

    EXPECT_EQ(decrypted, data);
}

TEST(CipherTest, EncryptDecryptExactBlock) {
    std::vector<uint8_t> data(32, 0xAB);
    keyhash              key = make_key(555);

    auto encrypted = encrypt(data, key);
    auto decrypted = decrypt(encrypted, key);

    EXPECT_EQ(decrypted, data);
}

TEST(CipherTest, PaddingIsRemoved) {
    std::vector<uint8_t> data(31, 0x01);  // Will require 1 byte of padding
    keyhash              key = make_key(777);

    auto encrypted = encrypt(data, key);
    ASSERT_EQ(encrypted.size() % 32, 0u);

    auto decrypted = decrypt(encrypted, key);
    EXPECT_EQ(decrypted, data);
}

TEST(CipherTest, DecryptInvalidSizeReturnsEmpty) {
    std::vector<uint8_t> invalid_data(15, 0xFF);  // Not a multiple of 32
    keyhash              key = make_key(888);

    auto decrypted = decrypt(invalid_data, key);
    EXPECT_TRUE(decrypted.empty());
}

TEST(CipherTest, DifferentKeysProduceDifferentCiphertext) {
    std::vector<uint8_t> data = {10, 20, 30, 40, 50, 60, 70, 80};
    keyhash              key1 = make_key(1);
    keyhash              key2 = make_key(2);

    auto encrypted1 = encrypt(data, key1);
    auto encrypted2 = encrypt(data, key2);

    EXPECT_NE(encrypted1, encrypted2);
}

TEST(CipherTest, CipherBlockIsReversible) {
    std::array<uint8_t, 32> block;
    for (size_t i = 0; i < 32; ++i) block[i] = static_cast<uint8_t>(i);

    keyhash key = make_key(999);
    auto    ops = get_operations(key.bits);
    auto    enc = cipher_block(block, ops);

    std::vector<operation> rev_ops = ops;
    std::reverse(rev_ops.begin(), rev_ops.end());
    auto dec = decipher_block(enc, rev_ops);

    EXPECT_EQ(std::vector<uint8_t>(dec.begin(), dec.end()),
              std::vector<uint8_t>(block.begin(), block.end()));
}
