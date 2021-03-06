// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_TYPES_H_
#define UTIL_TYPES_H_

#include <stdint.h>

#include <utility>
#include <vector>

class PDICDatafield;

typedef unsigned char byte;
typedef uint16_t unichar;
typedef unsigned int uint;

// typedef byte* byteptr;
// typedef byteptr* lookup_result;
// typedef lookup_result* lookup_result_ptr;
// typedef std::vector<lookup_result> lookup_result_vec;
// typedef lookup_result_vec (lookup_proc)(byte* needle, int needle_len);

typedef void (action_proc)(PDICDatafield* datafield);

typedef std::pair<bool, std::pair<int, int> > bsearch_result_t;

#endif  // UTIL_TYPES_H_
