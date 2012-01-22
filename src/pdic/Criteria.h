// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDIC_CRITERIA_H_
#define PDIC_CRITERIA_H_

#include <re2/re2.h>

#include "util/types.h"

class PDICDatafield;

class Criteria {
 public:
  Criteria(byte *needle_utf8, int target_charcode, bool exact_match);
  ~Criteria();

 public:
  bool match(PDICDatafield *datafield);

  RE2  *re2_pattern;
  byte *needle_utf8, *needle_for_index_utf8;
  byte *needle, *needle_for_index;
  int   needle_size, needle_size_for_index;
  bool  exact_match;
};

#endif  // PDIC_CRITERIA_H_
