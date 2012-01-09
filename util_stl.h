#ifndef UTIL_STL_H
#define UTIL_STL_H

#include <string>
#include <vector>

std::vector<std::string> split(std::string str, int delim=' ');
std::string join(std::vector<std::string> strs, const std::string &delim="");

#endif
