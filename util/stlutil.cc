// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./stlutil.h"

#include <sstream>
#include <string>
#include <vector>

std::string strlower(const std::string& str) {
  unsigned int len = str.size();
  std::vector<char> buf(len);
  for (unsigned int i = 0; i < len; ++i)
    buf[i] = isupper(str[i]) ? tolower(str[i]) : str[i];
  return std::string(buf.begin(), buf.end());
}

std::vector<std::string> split(std::string str, int delim) {
  std::vector<std::string> result;

  const char *s = str.c_str();
  if (delim == ' ') {
    for (const char *p = s; *p; p++) {
      if (*p == delim) {
        s++;
      } else {
        break;
      }
    }
    if (!*s) return result;

    for (const char *p = s; *p; p++) {
      if (*p == delim) {
        if (s < p) {
          std::string a(s, p-s);
          result.push_back(a);
        }
        s = p + 1;
      }
    }
    if (*s) result.push_back(s);
  } else {
    for (const char *p = s; *p; p++) {
      if (*p == delim) {
        std::string a(s, p-s);
        result.push_back(a);
        s = p + 1;
        if (*s == '\0') result.push_back("");
      }
    }
    if (*s) result.push_back(s);
  }

  return result;
}

std::vector<std::string> split(std::string str, std::string delim)
{
  std::vector<std::string> result;

  if (str.length() == 0) return result;

  if (delim.length() == 0) {
    int len = str.length();
    result.resize(len);
    for (int i = 0; i < len; ++i) result[i] = str.substr(i, 1);
    return result;
  }

  std::string::size_type since = 0, at;
  while ((at = str.find(delim, since)) != std::string::npos) {
    result.push_back(str.substr(since, at-since));
    since = at + delim.length();
  }
  result.push_back(str.substr(since));

  return result;
}

std::string join(std::vector<std::string> strs, const std::string& delim) {
  int n = strs.size();
  if (n == 0) return "";

  std::stringstream ss;
  ss << strs[0];
  for (int i = 1; i < n; ++i)
    ss << delim << strs[i];

  return ss.str();
}

std::string pluralize_if_plural(const std::string& singular_form, int number) {
  return singular_form + (number >= 2 ? "s" : "");
}

std::string number_with_unit(int number, const std::string& unit_str) {
  std::stringstream ss;
  ss << number << " " << pluralize_if_plural(unit_str, number);
  return ss.str();
}

std::ostream& operator<<(std::ostream& s, std::vector<std::string> v) {
  int cnt = v.size();
  s << "[ ";
  for (int i = 0; i < cnt; i++) {
    if (i > 0) s << ", ";
    s << '"' << v[i] << '"';
  }
  return s << " ]  // " << number_with_unit(cnt, "item");
}
