#ifndef UTIL_STL_H
#define UTIL_STL_H

#include <string>
#include <vector>
#include <utility>

#define rep(var,n)  for(int var=0;var<(n);var++)
#define traverse(c,i)  for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); i++)
#define all(c)  (c).begin(),(c).end()
#define found(s,e)  ((s).find(e)!=(s).end())

std::vector<std::string> split(std::string str, int delim=' ');
std::string join(std::vector<std::string> strs, const std::string &delim="");

#endif
