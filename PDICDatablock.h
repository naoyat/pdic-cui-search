#ifndef PDIC_DATABLOCK_H
#define PDIC_DATABLOCK_H

#include "PDICIndex.h"
#include "Criteria.h"

#include <cstdio>

class PDICDatablock {
private:
  //  PDICHeader *header;
  PDICIndex *_index;
  unsigned char *_datablock_buf;
  int _datablock_buf_size;
  bool _is4byte;
  bool _isAligned;

public:
  PDICDatablock(FILE *fp, PDICIndex *index, int ix);
  ~PDICDatablock();

  void iterate(action_proc *action, Criteria *criteria = NULL);
  //void dump();
};

#endif
