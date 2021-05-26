#ifndef PROJ2_UTILS_H
#define PROJ2_UTILS_H

#include <string>

void print_error_msg_and_exit(std::string msg);

void print_invalid_value_msg_and_exit(std::string msg);

bool is_player_name_valid(const std::string &name);

#endif  // PROJ2_UTILS_H
