#include <getopt.h>
#include <iostream>

#include "options.h"

options_t parse_options(int argc, char *const argv[]) {
    options_t options;
    int opt;
    while ((opt = getopt(argc, argv, "+:n:p:i:r:")) != -1) {
        switch (opt) {
            case 'n':
                options.insert({"player_name", optarg});
                break;
            case 'p':
                options.insert({"game_server_port", optarg});
                break;
            case 'i':
                options.insert({"gui_server", optarg});
                break;
            case 'r':
                options.insert({"gui_server_port", optarg});
                break;
            case ':': {
                std::cerr << "Option " << (char)optopt << " requires an option argument"
                          << std::endl;
                std::cerr << "Usage: ./screen-worms-client game_server [-n player_name] [-p "
                             "n] [-i gui_server] [-r n]"
                          << std::endl;
                exit(EXIT_FAILURE);
            }
            case '?': {
                std::cerr << "Unrecognized option: " << (char)optopt << std::endl;
                std::cerr << "Usage: ./screen-worms-client game_server [-n player_name] [-p "
                             "n] [-i gui_server] [-r n]"
                          << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
    if (optind < argc) {
        std::cerr << "Unnecessary extra argument: " << argv[optind] << std::endl;
        exit(EXIT_FAILURE);
    }

    if (options.find("player_name") == options.end()) {
        options["player_name"] = "";
    }
    if (options.find("game_server_port") == options.end()) {
        options["game_server_port"] = "2021";
    }
    if (options.find("gui_server") == options.end()) {
        options["gui_server"] = "localhost";
    }
    if (options.find("gui_server_port") == options.end()) {
        options["gui_server_port"] = "20210";
    }

    return options;
}
