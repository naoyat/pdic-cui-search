// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SRC_SEARCH_H_
#define SRC_SEARCH_H_

#include <utility>

#include "./types.h"

std::pair<byte*, int*> concat_strings(byte* strings[],
                                      int string_count,
                                      int end_marker = 0);
std::pair<int*, int> make_full_suffix_array(byte* buf, int buf_size);
std::pair<int*, int> make_light_suffix_array(byte* buf, int buf_size);

bsearch_result_t search(byte* buf, int* offsets, int offsets_len,
                        byte* needle, bool exact_match);

#endif  // SRC_SEARCH_H_
