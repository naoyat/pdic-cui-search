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
  //  PDICHeader *header;
  PDICIndex *_index;
  int _ix;
  //unsigned char *_datablock_buf;
  //int _datablock_offset, _datablock_buf_size;
  bool _v6index;
  bool _is4byte;
  bool _isAligned;

public:
  PDICDatablock(byte* datablock_start, PDICIndex *index, int ix);
  ~PDICDatablock() {}

  void iterate(action_proc *action, Criteria *criteria = NULL);
  //void dump();
};

#endif
