// Copyright 2012-2013 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDIC_LOOKUP_RESULT_H_
#define PDIC_LOOKUP_RESULT_H_

#include <vector>

#include "util/types.h"  // byte

class PDICDatafield;

class lookup_result {
 public:
           lookup_result(byte *entry, byte *jword,
                         byte *example, byte *pron,
                         int level=-1);
  explicit lookup_result(PDICDatafield* datafield);
           lookup_result(byte* dict_buf[], int start_pos[], int level=-1);
  ~lookup_result();

 public:
  byte *entry, *jword, *example, *pron;
  int level;
};

// typedef lookup_result* lookup_result_ptr;
typedef std::vector<lookup_result*> lookup_result_vec;
typedef lookup_result_vec (lookup_proc)(byte* needle, int needle_len);

#define F_COUNT    4

#define F_ENTRY    0
#define F_JWORD    1
#define F_EXAMPLE  2
#define F_PRON     3


#endif  // PDIC_LOOKUP_RESULT_H_
