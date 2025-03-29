#include <getopt.h>

#include <iostream>

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

    struct option long_options [] = {
        {"verbose", no_argument,       0, 'v'},
        {"output",  required_argument, 0, 'o'},
        {"input",   required_argument, 0, 'i'},
        {"encrypt", no_argument,       0, 'e'},
        {"decrypt", no_argument,       0, 'd'},
        {0,         0,                 0, 0  },
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "voied", long_options, nullptr))
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
        }
    }

    std::cout << "Hello, from lea!\n";
}
