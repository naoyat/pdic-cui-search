#ifndef PDICCUISEARCH_PDICHEADER_H_
#define PDICCUISEARCH_PDICHEADER_H_

#include <stdio.h>

#include "types.h"


#define OLDDIC  1 // PDIC ver 1.x : OLDDIC
#define NEWDIC1 2 // PDIC ver 2.x : NEWDIC1
#define NEWDIC2 3 // PDIC ver 3.x : NEWDIC2
#define HYPER4  4 // PDIC ver 4.x : NEWDIC3 Hyper
#define HYPER5  5 // PDIC ver 5.x : NEWDIC3 Hyper (aligned)
#define HYPER6  6 // PDIC ver 6.x : NEWDIC3 Hyper (aligned), BOCU-1

#define L_HEADERNAME 100
#define L_DICTITLE    40

//[0, 167)
#define OFS_HEADERNAME     0 // char[ L_HEADERNAME ]
#define OFS_DICTITLE     100 // char[ L_DICTITLE ]
#define OFS_VERSION      140 // short
#define OFS_LWORD        142 // short
#define OFS_LJAPA        144 // short
#define OFS_BLOCK_SIZE   146 // short
#define OFS_INDEX_BLOCK  148 // short
#define OFS_HEADER_SIZE  150 // short
#define OFS_INDEX_SIZE   152 // short, 4〜ushort
#define OFS_EMPTY_BLOCK  154 // short
#define OFS_NINDEX       156 // short
#define OFS_NBLOCK       158 // short
#define OFS_NWORD        160 // ulong
#define OFS_DICORDER     164 // uchar
#define OFS_DICTYPE      165 // uchar
#define OFS_ATTRLEN      166 // uchar

// [167, 172) HYPER5から4バイトアラインメントが考慮された
#define OFS_NEWDIC2_OLENUMBER  167 // long
#define OFS_NEWDIC2_OS         171 // uchar

#define OFS_HYPER5_OS          167 // long
#define OFS_HYPER5_OLENUMBER   168 // uchar

// [172, 182) 言語ID（NEWDIC2〜。HYPER6で廃止されてる）
#define OFS_LID_WORD   172 // ushort
#define OFS_LID_JAPA   174 // ushort
#define OFS_LID_EXP    176 // ushort
#define OFS_LID_PRON   178 // ushort
#define OFS_LID_OTHER  180 // ushort

// [182, 208)
#define OFS_HYPER4_EXTHEADER    182 // ulong
#define OFS_HYPER4_EMPTY_BLOCK2 186 // long
#define OFS_HYPER4_NINDEX2      190 // long
#define OFS_HYPER4_NBLOCK2      194 // long
#define OFS_HYPER4_INDEX_BLKBIT 198 // uchar
#define OFS_HYPER4_RESERVED     199 // uchar[8+1]

#define OFS_HYPER5_INDEX_BLKBIT 182 // uchar
#define OFS_HYPER5_DUMMY0       183 // uchar
#define OFS_HYPER5_EXTHEADER    184 // ulong
#define OFS_HYPER5_EMPTY_BLOCK2 188 // long
#define OFS_HYPER5_NINDEX2      192 // long
#define OFS_HYPER5_NBLOCK2      196 // long

// [200, 208)
//#define OFS_HYPER5_RESERVED     200 // uchar[8]
#define OFS_HYPER6_CYPT         200 // uchar[8]

// [208, 256)
#define OFS_UPDATE_COUNT        208 // ulong; Hyper4〜
#define OFS_HYPER4_CHARCODE     212 // uchar
//#define OFS_HYPER5_DUMMY00      212 // uchar[4]
#define OFS_HYPER5_DICIDENT     216 // uchar[8]
#define OFS_HYPER6_DEREFID      224 // uchar[8]
//#define OFS_HYPER4_DUMMY        213 // char[43]
//#define OFS_HYPER5_DUMMY        232 // char[24]


class PDICHeader {
private:
  byte *filemem;

  int _version;
  int _major_version, _minor_version;

public:
  PDICHeader(byte *filemem);
  ~PDICHeader();

public:
  byte* headername();
  byte* dictitle();
  int   version();
  int   major_version();
  int   minor_version();
  int   lword();
  int   ljapa();
  int   block_size();
  int   index_block();
  int   header_size();
  int   index_size();
  int   empty_block();
  int   nindex();
  int   nblock();
  unsigned int nword();
  int   dicorder();
  int   dictype();
  int   attrlen();
  int   olenumber();
  int   os();
  int   lid_word();
  int   lid_japa();
  int   lid_exp();
  int   lid_pron();
  int   lid_other();
  int   index_blkbit(); // 0:16bit, 1:32bit
  unsigned int extheader();
  byte* cypt();
  unsigned int update_count();
  byte* dicident();
  byte* derefid();

public:
  bool  isAligned();
  bool  isBOCU1(); // BOCU-1辞書か？

  void  dump();
};

#endif // PDICCUISEARCH_PDICHEADER_H_
