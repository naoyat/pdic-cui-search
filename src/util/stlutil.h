// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_STLUTIL_H_
#define UTIL_STLUTIL_H_

#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#define rep(var, n)     for (int var = 0; var < (n); var++)
#define traverse(c, i)  \
  for (typeof((c).begin()) i = (c).begin(); i != (c).end(); i++)
#define all(c)          (c).begin(), (c).end()
#define found(s, e)     ((s).find(e) != (s).end())

std::vector<std::string> split(std::string str, int delim=' ');
std::string join(std::vector<std::string> strs, const std::string &delim="");

std::string pluralize_if_plural(const std::string& singular_form, int number);
std::string number_with_unit(int number, const std::string& unit_str);
std::ostream& operator<<(std::ostream& s, std::vector<std::string> v);

template <typename T>
    std::ostream& operator<<(std::ostream& s, std::vector<T> v) {
  int cnt = v.size();
  s << "[ ";
  for (int i = 0; i < cnt; i++) {
    if (i > 0) s << ", ";
    s << v[i];
  }
  return s << " ]  // " << number_with_unit(cnt, "item");
}

template <typename T>
    std::ostream& operator<<(std::ostream &s, std::list<T> ls) {
  int cnt = 0;
  s << "( ";
  for (typeof(ls.begin()) it = ls.begin(); it != ls.end(); it++) {
    if (it != ls.begin()) s << ", ";
    s << *it;
    cnt++;
  }
  return s << " )  // " << number_with_unit(cnt, "item");
}

template <typename T>
    std::ostream& operator<<(std::ostream &s, std::deque<T> st) {
  int cnt = st.size();
  s << "[ ";
  for (typeof(st.begin()) it = st.begin(); it != st.end(); it++) {
    if (it != st.begin()) s << ", ";
    s << *it;
  }
  return s << " ]  // " << number_with_unit(cnt, "item");
}

template <typename T1, typename T2>
    std::ostream& operator<<(std::ostream &s, std::map<T1, T2> m) {
  int cnt = m.size();
  s << "{ ";
  for (typeof(m.begin()) it = m.begin(); it != m.end(); it++) {
    if (it != m.begin()) s << ", ";
    s << it->first << " => " << it->second;
  }
  return s << " }  // " << number_with_unit(cnt, "item");
}

template <typename T>
    std::ostream& operator<<(std::ostream &s, std::set<T> st) {
  int cnt = st.size();
  s << "[ ";
  for (typeof(st.begin()) it = st.begin(); it != st.end(); it++) {
    if (it != st.begin()) s << ", ";
    s << *it;
  }
  return s << " ]  // " << number_with_unit(cnt, "item");
}

template <typename T1, typename T2>
    std::ostream& operator<<(std::ostream &s, std::pair<T1, T2> p) {
  return s << "(" << p.first << "," << p.second << ")";
}

#endif  // UTIL_STLUTIL_H_
