// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDIC_LOOKUP_H_
#define PDIC_LOOKUP_H_

#include <re2/re2.h>

#include <set>

#include "util/types.h"

#define LOOKUP_AUTO               0x0000
#define LOOKUP_PDIC_INDEX         0x0001
#define LOOKUP_SARRAY             0x0002
#define LOOKUP_REGEXP             0x0004
#define LOOKUP_FROM_ALL           0x0008

#define LOOKUP_MATCH_FORWARD      0x0100
#define LOOKUP_MATCH_BACKWARD     0x0200
#define LOOKUP_EXACT_MATCH        (LOOKUP_MATCH_FORWARD | LOOKUP_MATCH_BACKWARD)
#define LOOKUP_CASE_SENSITIVE     0x1000

#define LOOKUP_PDIC_MATCH_FORWARD  (LOOKUP_PDIC_INDEX | LOOKUP_MATCH_FORWARD)

//
lookup_result_vec _pdic_match_forward_lookup(byte *needle, int flags);
lookup_result_vec _sarray_lookup(byte *needle, int flags);
lookup_result_vec _regexp_lookup(RE2 *current_pattern, int flags);
lookup_result_vec _full_lookup(byte *needle, int flags);
lookup_result_vec _exact_lookup(byte *needle, int flags);

// 汎用lookup() - フラグで検索方法を判定
void lookup(byte *needle, int flags);

/*
inline void pdic_match_forward_lookup(byte *needle,
                                      bool case_sensitive = false) {
  int flags = LOOKUP_PDIC_MATCH_FORWARD;
  if (case_sensitive) flags |= LOOKUP_CASE_SENSITIVE;
  lookup(needle, flags);
}
inline void exact_lookup(byte *needle, bool case_sensitive = true) {
  int flags = LOOKUP_PDIC_MATCH_FORWARD | LOOKUP_EXACT_MATCH;
  if (case_sensitive) flags |= LOOKUP_CASE_SENSITIVE;
  lookup(needle, flags);
}
inline void sarray_lookup(byte *needle) {
  lookup(needle, LOOKUP_SARRAY);
}
inline void regexp_lookup(byte *needle) {
  lookup(needle, LOOKUP_REGEXP);
}
inline void full_lookup(byte *needle) {
  lookup(needle, LOOKUP_FROM_ALL);
}
*/

void render_current_result();
void render_current_result(const std::set<int>& range);

#endif  // PDIC_LOOKUP_H_
