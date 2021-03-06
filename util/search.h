// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_SEARCH_H_
#define UTIL_SEARCH_H_

#include <utility>

#include "util/types.h"

std::pair<byte*, int*> concat_strings(byte* strings[],
                                      int string_count,
                                      int end_marker = 0);
std::pair<int*, int> make_full_suffix_array(byte* buf, int buf_size);
std::pair<int*, int> make_light_suffix_array(byte* buf, int buf_size);

bsearch_result_t search(byte* buf, int* offsets, int offsets_len,
                        byte* needle, bool exact_match, int offset_mag = 1);

#endif  // UTIL_SEARCH_H_
