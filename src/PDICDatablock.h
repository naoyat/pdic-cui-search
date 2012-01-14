#ifndef PDIC_DATABLOCK_H
#define PDIC_DATABLOCK_H

#include <cstdio>
#include "types.h"

class Criteria;
class PDICIndex;

class PDICDatablock {
private:
  byte *filemem, *datablock_start;
  int  datablock_size;
  PDICIndex *_index;
  int _ix;

  bool _v6index;
  bool _is4byte;
  bool _isAligned;

public:
  PDICDatablock(byte* datablock_start, PDICIndex *index, int ix);
  ~PDICDatablock() {}

  void iterate(action_proc *action, Criteria *criteria = NULL);
};

#endif
