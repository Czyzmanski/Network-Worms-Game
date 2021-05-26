#ifndef PROJ2_OPTIONS_H
#define PROJ2_OPTIONS_H

#include <map>
#include <string>

using options_t = std::map<const std::string, std::string>;

options_t parse_options(int argc, char *const argv[]);

#endif  // PROJ2_OPTIONS_H
