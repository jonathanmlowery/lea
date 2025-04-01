#include <getopt.h>

#include <bitset>
#include <iostream>

#include "cipher.hpp"
#include "keyhash.hpp"

int main(int argc, char** argv) {
    bool        verbose = false;
    Mode        mode    = UNSET;
    std::string input_file;
    std::string output_file;
    std::string key_str;

    struct option long_options [] = {
        {"verbose", no_argument,       0, 'v'},
        {"output",  required_argument, 0, 'o'},
        {"input",   required_argument, 0, 'i'},
        {"encrypt", no_argument,       0, 'e'},
        {"decrypt", no_argument,       0, 'd'},
        {"key",     required_argument, 0, 'k'},
        {0,         0,                 0, 0  },
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "voiedk", long_options, nullptr))
           != -1) {
        switch (opt) {
            case 'e':
                mode = ENCRYPT;
                break;

            case 'd':
                mode = DECRYPT;
                break;

            case 'v':
                verbose = true;
                break;

            case 'i':
                input_file = optarg;
                break;

            case 'o':
                output_file = optarg;
                break;

            case 'k':
                key_str = optarg;
                break;
        }
    }

    std::bitset<256> original1 = lea::bitify_str("abc");
    std::bitset<256> original2 = lea::bitify_str("abcd");
    lea::keyhash     hashed1   = lea::gen_keyhash(original1, 256);
    lea::keyhash     hashed2   = lea::gen_keyhash(original2, 256);
    std::bitset<256> diff      = hashed1.bits ^ hashed2.bits;
    int              flips     = diff.count();
    // std::bitset<512> expanded = lea::bit_interleaving_expand(original,
    // 7); std::bitset<256> compacted =
    // lea::sequential_bit_compact(expanded);

    std::cout << "Original1 bits: \n" << original1 << '\n';
    std::cout << "Keyhash1 bits: \n" << hashed1.hex_str() << '\n';
    std::cout << "Original2 bits: \n" << original2 << '\n';
    std::cout << "Keyhash2 bits: \n" << hashed2.hex_str() << '\n';
    // std::cout << "Expanded bits: \n" << expanded << '\n';
    // std::cout << "Compacted bits: \n" << compacted << '\n';
    std::cout << "Flips: " << flips << '\n';
}
