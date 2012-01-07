#ifndef __PDIC_HEADER_H
#define __PDIC_HEADER_H

#include <cstdio>
#include "util.h"

#define OFS_HEADERNAME     0
#define OFS_DICTITLE     100
#define OFS_VERSION      140
#define OFS_LWORD        142
#define OFS_LJAPA        144
#define OFS_BLOCK_SIZE   146
#define OFS_INDEX_BLOCK  148
#define OFS_HEADER_SIZE  150
#define OFS_INDEX_SIZE   152
#define OFS_EMPTY_BLOCK  154
#define OFS_NINDEX       156
#define OFS_NBLOCK       158
#define OFS_NWORD        160
#define OFS1024_DICORDER     164
#define OFS1024_DICTYPE      165
#define OFS1024_ATTRLEN      166
#define OFS1024_OS           167
#define OFS1024_OLENUMBER    168
#define OFS1024_DUMMY_LID    172
#define OFS1024_INDEX_BLKBIT 182
#define OFS1024_DUMMY0       183
#define OFS1024_EXTHEADER    184
#define OFS1024_EMPTY_BLOCK2 188
#define OFS1024_NINDEX2      192
#define OFS1024_NBLOCK2      196
#define OFS1024_CYPT         200
#define OFS1024_UPDATE_COUNT 208
#define OFS1024_DUMMY00      212
#define OFS1024_DICIDENT     216
#define OFS1024_DEREFID      224
#define OFS1024_DUMMY        232

class PDICHeader {
private:
  unsigned char buf[1024];
  int _version;

public:
  PDICHeader(FILE *fp);
  ~PDICHeader();

  void dump();

public:
  unsigned char* headername() { return buf + OFS_HEADERNAME; }
  unsigned char* dictitle() { return buf + OFS_DICTITLE; }
  unsigned int version() { return ushortval(buf + OFS_VERSION); }
  unsigned int lword() { return ushortval(buf + OFS_LWORD); }
  unsigned int ljapa() { return ushortval(buf + OFS_LJAPA); }
  unsigned int block_size() { return ushortval(buf + OFS_BLOCK_SIZE); } // 1024
  unsigned int index_block() { return ushortval(buf + OFS_INDEX_BLOCK); }
  unsigned int header_size() { return ushortval(buf + OFS_HEADER_SIZE); } // 1024
  int index_size() {
    //return shortval(buf + OFS_INDEX_SIZE);
    return index_block() * block_size();
  }
  int empty_block() {
    //return shortval(buf + OFS_EMPTY_BLOCK);
    return longval(buf + OFS1024_EMPTY_BLOCK2);
  }
  unsigned int nindex() {
    //return shortval(buf + OFS_NINDEX);
    return ulongval(buf + OFS1024_NINDEX2);
  }
  unsigned int nblock() {
    //return shortval(buf + OFS_NBLOCK);
    return ulongval(buf + OFS1024_NBLOCK2);
  }
  unsigned int nword() { return ulongval(buf + OFS_NWORD); }
  int dicorder() { return byteval(buf + OFS1024_DICORDER); }
  int dictype() { return byteval(buf + OFS1024_DICTYPE); }
  int attrlen() { return byteval(buf + OFS1024_ATTRLEN); }
  int os() { return byteval(buf + OFS1024_OS); }
  int olenumber() { return longval(buf + OFS1024_OLENUMBER); }
  int index_blkbit() { return byteval(buf + OFS1024_INDEX_BLKBIT); }
  unsigned int extheader() { return ulongval(buf + OFS1024_EXTHEADER); }

  unsigned char *cypt() { return buf + OFS1024_CYPT; }
  unsigned int update_count() { return ulongval(buf + OFS1024_UPDATE_COUNT); }
  unsigned char *dicident() { return buf + OFS1024_DICIDENT; }
};

#endif

