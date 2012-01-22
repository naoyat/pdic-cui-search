// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./stlutil.h"

#include <sstream>
#include <string>
#include <vector>

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
