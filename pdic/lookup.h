// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDIC_LOOKUP_H_
#define PDIC_LOOKUP_H_

#include <re2/re2.h>

#include <set>

#include "util/types.h"

#define LOOKUP_NORMAL       0x0010
#define LOOKUP_SARRAY       0x0100
#define LOOKUP_REGEXP       0x1000
#define LOOKUP_EXACT_MATCH  0x0001
#define LOOKUP_FULL         0x1111

//
lookup_result_vec _normal_lookup(byte *needle, int needle_len = 0);
lookup_result_vec _sarray_lookup(byte *needle, int needle_len = 0);
lookup_result_vec _regexp_lookup(RE2 *current_pattern);
lookup_result_vec _full_lookup(byte *needle, int needle_len = 0);

void lookup(byte *needle, int needle_len, int flag);

inline void normal_lookup(byte *needle, int needle_len = 0) {
  lookup(needle, needle_len, LOOKUP_NORMAL);
}
inline void sarray_lookup(byte *needle, int needle_len = 0) {
  lookup(needle, needle_len, LOOKUP_SARRAY);
}
inline void regexp_lookup(byte *needle, int needle_len = 0) {
  lookup(needle, needle_len, LOOKUP_REGEXP);
}
inline void full_lookup(byte *needle, int needle_len = 0) {
  lookup(needle, needle_len, LOOKUP_FULL);
}

void default_lookup(byte *needle, int needle_len = 0);

void render_current_result();
void render_current_result(const std::set<int>& range);

#endif  // PDIC_LOOKUP_H_
