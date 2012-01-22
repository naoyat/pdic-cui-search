#include "stlutil.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


std::vector<std::string> split(std::string str, int delim)
{
  std::vector<std::string> result;

  const char *s = str.c_str();
  if (delim == ' ') {
    for (const char *p=s; *p; p++) {
      if (*p == delim) s++;
      else break;
    }
    if (!*s) return result;

    for (const char *p=s; *p; p++) {
      if (*p == delim) {
        if (s < p) {
          std::string a(s,p-s);
          result.push_back(a);
        }
        s = p + 1;
      }
    }
    if (*s) result.push_back(s);
  } else {
    for (const char *p=s; *p; p++) {
      if (*p == delim) {
        std::string a(s,p-s);
        result.push_back(a);
        s = p + 1;
        if (*s == '\0') result.push_back("");
      }
    }
    if (*s) result.push_back(s);
  }

  return result;
}

std::string join(std::vector<std::string> strs, const std::string& delim)
{
  int n=strs.size(); if (n==0) return "";
  std::stringstream ss;
  ss << strs[0];
  for(int i=1;i<n;i++) { ss << delim << strs[i]; }
  return ss.str();
}

/**
// 文字列をdelimにする版。あまり使わないけど
vector<string> split(string str, string delim)
{
  vector<string> result;

  if (str.length() == 0) return result;

  if (delim.length() == 0) {
    int len = str.length();
    result.resize(len);
    for (int i=0; i<len; i++) result[i] = str.substr(i,1);
    return result;
  }

  int since = 0, at;
  while ((at = str.find(delim, since)) != string::npos) {
    result.push_back(str.substr(since, at-since));
    since = at + delim.length();
  }
  result.push_back(str.substr(since));

  return result;
}
**/

std::string pluralize_if_plural(const std::string &singular_form, int number)
{
  return singular_form + (number >= 2 ? "s" : "");
}

std::string number_with_unit(int number, const std::string &unit_str)
{
  std::stringstream ss;
  // ss << number << " " << unit_str << (number >= 2 ? "s" : "");
  ss << number << " " << pluralize_if_plural(unit_str,number);
  return ss.str();
}

std::ostream& operator<<(std::ostream &s, std::vector<std::string> v)
{
  int cnt = v.size();
  s << "[ ";
  for (int i=0; i<cnt; i++) {
    if (i > 0) s << ", ";
    s << '"' << v[i] << '"';
  }
  return s << " ]  // " << number_with_unit(cnt,"item");
}
