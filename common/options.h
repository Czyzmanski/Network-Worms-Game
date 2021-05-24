#ifndef PROJ2_OPTIONS_H
#define PROJ2_OPTIONS_H

#include <string>
#include <map>

using options_t = std::map<const std::string, const std::string>;

options_t parse_options(int argc, const char *argv[], const char *optstring);

#endif //PROJ2_OPTIONS_H
