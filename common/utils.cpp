#include <cstring>
#include <iostream>

#include "utils.h"

void print_error_msg_and_exit(std::string msg) {
    std::cerr << msg << ": " << std::strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}

void print_invalid_value_msg_and_exit(std::string msg) {
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
}

bool is_player_name_valid(const std::string &name) {
    if (name.length() > 20) {
        return false;
    }
    for (auto &c : name) {
        if (c < 33 || c > 126) {
            return false;
        }
    }

    return true;
}
