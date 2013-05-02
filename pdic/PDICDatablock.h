// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDIC_PDICDATABLOCK_H_
#define PDIC_PDICDATABLOCK_H_

#include <stdio.h>

#include "util/types.h"

class Criteria;
class PDICHeader;
class PDICIndex;

class PDICDatablock {
 public:
  PDICDatablock(byte*         filemem,
                PDICIndex*    index,
                int           ix);
  PDICDatablock(byte*         filemem,
                unsigned int  datablock_start_offset,
                PDICHeader*   header);
  ~PDICDatablock() {}

 private:
  void init(byte*         filemem,
            unsigned int  datablock_start_offset,
            PDICHeader*   header);

 public:
  void iterate(action_proc* action, Criteria* criteria = NULL);

 private:
  byte* filemem;
  byte* datablock_start;
  int   datablock_size;
  // PDICIndex* _index;
  PDICHeader* _header;
  // int   _ix;

  bool  _v6index;
  bool  _is4byte;
  bool  _isAligned;
};

#endif  // PDIC_PDICDATABLOCK_H_
