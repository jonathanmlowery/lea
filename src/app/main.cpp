#include <getopt.h>

#include <bitset>
#include <iostream>

#include "keyhash.hpp"

enum Mode {
    DECRYPT,
    UNSET,
    ENCRYPT,
};

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

    std::bitset<256> original = std::bitset<256>(8'567'498'930'133'322);
    std::bitset<512> expanded = lea::bit_interleaving_expand(original, 53);
    std::bitset<256> compacted = lea::sequential_bit_compact(expanded);

    std::cout << "Original bits: \n" << original << '\n';
    std::cout << "Expanded bits: \n" << expanded << '\n';
    std::cout << "Compacted bits: \n" << compacted << '\n';
}
