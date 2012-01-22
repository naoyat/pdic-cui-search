// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SRC_PDICDATABLOCK_H_
#define SRC_PDICDATABLOCK_H_

#include <stdio.h>

#include "./types.h"

class Criteria;
class PDICIndex;

class PDICDatablock {
 public:
  PDICDatablock(byte* datablock_start, PDICIndex* index, int ix);
  ~PDICDatablock() {}

 public:
  void iterate(action_proc* action, Criteria* criteria = NULL);

 private:
  byte* filemem;
  byte* datablock_start;
  int   datablock_size;
  PDICIndex* _index;
  int   _ix;

  bool  _v6index;
  bool  _is4byte;
  bool  _isAligned;
};

#endif  // SRC_PDICDATABLOCK_H_
