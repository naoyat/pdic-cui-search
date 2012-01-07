#ifndef __PDIC_DATABLOCK_H
#define __PDIC_DATABLOCK_H

#include "PDICIndex.h"
#include "Criteria.h"

#include <cstdio>

//typedef bool (criteria_proc)(unsigned char*,unsigned char*);
typedef void (action_proc)(unsigned char*,unsigned char*);

class PDICDatablock {
private:
  //  PDICHeader *header;
  PDICIndex *index;
  unsigned char *datablock_buf;
  int datablock_buf_size;
  bool is_4_byte;

public:
  PDICDatablock(FILE *fp, PDICIndex *index, int ix);
  ~PDICDatablock();

  void iterate(action_proc *action, Criteria *criteria = NULL);
  void dump();
};

#endif
