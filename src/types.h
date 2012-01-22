// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDICCUISEARCH_TYPES_H_
#define PDICCUISEARCH_TYPES_H_

#include <utility>
#include <vector>


class PDICDatafield;

typedef unsigned char byte;
typedef unsigned short unichar;
typedef unsigned int uint;

typedef byte *byteptr;
typedef byteptr *lookup_result;
typedef lookup_result *lookup_result_ptr;
typedef std::vector<lookup_result> lookup_result_vec;
typedef lookup_result_vec (lookup_proc)(byte *needle, int needle_len);

typedef void (action_proc)(PDICDatafield*);

typedef std::pair<bool,std::pair<int,int> > bsearch_result_t;

#endif // PDICCUISEARCH_TYPES_H_
