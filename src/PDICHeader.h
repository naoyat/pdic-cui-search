#ifndef PDIC_HEADER_H
#define PDIC_HEADER_H

#include <cstdio>
#include "util.h" // sNNval(), uNNval()

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
  unsigned char buf[1024]; // とは言うものの、ver6 でも256で足りる
  int _version;
  int _major_version, _minor_version;

public:
  PDICHeader(FILE *fp);
  ~PDICHeader();

  void dump();

public:
  unsigned char* headername() { return buf + OFS_HEADERNAME; }
  unsigned char* dictitle() { return buf + OFS_DICTITLE; }
  int version() { return s16val(buf + OFS_VERSION); }
  int lword() {
    // HYPER6.10: { 1024 }
    // HYPER6.00: { 248 }
    // HYPER4, HYPER5: { 248 }
    return s16val(buf + OFS_LWORD);
  }
  int ljapa() {
    // HYPER6.10: { 262144 } 未参照
    // HYPER6.00: { 10000 }
    // HYPER4, HYPER5: { 3000 }
    return s16val(buf + OFS_LJAPA);
  }
  int block_size() {
    // HYPER6: { 1024 }
    // HYPER4, HYPER5: { 256 }
    // NEWDIC2: { 64, 128, 256, 512 }*16
    // NEWDIC1: { 1024, 2048, 4096, 8192 }
    if (_major_version >= HYPER4)
      return u16val(buf + OFS_BLOCK_SIZE);
    else if (_major_version >= NEWDIC2)
      return 16 * s16val(buf + OFS_BLOCK_SIZE);
    else // NEWDIC
      return s16val(buf + OFS_BLOCK_SIZE);
  }
  int index_block() {
    if (_major_version >= HYPER4)
      return s16val(buf + OFS_INDEX_BLOCK);
    else // NEWDIC, NEWDIC2
      return 2048 * s16val(buf + OFS_INDEX_BLOCK);
  }
  int header_size() {
    // HYPER6: { 1024 }
    // HYPER4, HYPER5: { 256 }
    return s16val(buf + OFS_HEADER_SIZE); // 256, 1024(6〜)
  }
  int index_size() {
    if (_major_version >= HYPER4)
      return index_block() * block_size();
    else // NEWDIC, NEWDIC2
      return s16val(buf + OFS_INDEX_SIZE); // { 2048, 10240, 20480, 40960 }
  }
  int empty_block() {
    // -1 if not exists
    if (_major_version >= HYPER5)
      return s32val(buf + OFS_HYPER5_EMPTY_BLOCK2);
    else if (_major_version >= HYPER4)
      return s32val(buf + OFS_HYPER4_EMPTY_BLOCK2);
    else // NEWDIC, NEWDIC2
      return s16val(buf + OFS_EMPTY_BLOCK); // [0-4095]
  }
  int nindex() {
    if (_major_version >= HYPER5)
      return u32val(buf + OFS_HYPER5_NINDEX2);
    else if (_major_version >= HYPER4)
      return u32val(buf + OFS_HYPER4_NINDEX2);
    else // NEWDIC, NEWDIC2
      return s16val(buf + OFS_NINDEX); // <= 4096
  }
  int nblock() {
    if (_major_version >= HYPER5)
      return u32val(buf + OFS_HYPER5_NBLOCK2);
    else if (_major_version >= HYPER4)
      return u32val(buf + OFS_HYPER4_NBLOCK2);
    else // NEWDIC, NEWDIC2
      return s16val(buf + OFS_NBLOCK);
  }
  unsigned int nword() {
    return u32val(buf + OFS_NWORD);
  }
  int dicorder() {
    // { 0[, 1, 2, 3] }
    if (_major_version >= NEWDIC2)
      return buf[OFS_DICORDER];
    else
      return 0;
  }
  int dictype() {
    // HYPER6:  { 0x01 | 0x08 |[0x10]|[0x20]| 0x40 |[0x80]}
    // HYPER5:  { 0x01 | 0x08 | 0x10 |[0x20]| 0x40 | 0x80 }
    // HYPER4:  { 0x01 |      |[0x10]| 0x20 | 0x40 | 0x80 }
    // NEWDIC2: { 0x01 |      | 0x10 | 0x20 }
    if (_major_version >= NEWDIC2)
      return buf[OFS_DICTYPE];
    else
      return 0;
  }
  int attrlen() {
    // { 1 }
    return buf[OFS_ATTRLEN];
  }
  int olenumber() {
    if (_major_version >= HYPER5)
      return s32val(buf + OFS_HYPER5_OLENUMBER);
    else if (_major_version >= NEWDIC2)
      return s32val(buf + OFS_NEWDIC2_OLENUMBER);
    else
      return 0;
  }
  int os() {
    // HYPER6: { 0x20="BOCU-1 encoding" }
    // HYPER5: { 0, 1, 2, 3, 4, 0x10 }
    // HYPER4: { 0 }
    // NEWDIC2: { 0[, 1, 2] }
    if (_major_version >= HYPER5)
      return buf[OFS_HYPER5_OS];
    else if (_major_version >= NEWDIC2)
      return buf[OFS_NEWDIC2_OS];
    else
      return 0;
  }
  int lid_word() {
    return _major_version == NEWDIC2 ? u16val(buf + OFS_LID_WORD) : 0;
  }
  int lid_japa() {
    return _major_version == NEWDIC2 ? u16val(buf + OFS_LID_JAPA) : 0;
  }
  int lid_exp() {
    return _major_version == NEWDIC2 ? u16val(buf + OFS_LID_EXP) : 0;
  }
  int lid_pron() {
    return _major_version == NEWDIC2 ? u16val(buf + OFS_LID_PRON) : 0;
  }
  int lid_other() {
    return _major_version == NEWDIC2 ? u16val(buf + OFS_LID_OTHER) : 0;
  }
  int index_blkbit() {
    // 0:16bit, 1:32bit
    if (_major_version >= HYPER5)
      return buf[OFS_HYPER5_INDEX_BLKBIT];
    else if (_major_version == HYPER4)
      return buf[OFS_HYPER4_INDEX_BLKBIT];
    else
      return 0;
  }
  unsigned int extheader() {
    if (_major_version >= HYPER5)
      return u32val(buf + OFS_HYPER5_EXTHEADER);
    else if (_major_version == HYPER4)
      return u32val(buf + OFS_HYPER4_EXTHEADER);
    else
      return 0;
  }
  unsigned char *cypt() {
    if (_major_version >= HYPER6)
      return buf + OFS_HYPER6_CYPT;
    else
      return NULL;
  }
  unsigned int update_count() {
    if (_major_version >= HYPER4)
      return u32val(buf + OFS_UPDATE_COUNT);
    else
      return 0;
  }
  unsigned char *dicident() {
    if (_major_version >= HYPER5)
      return buf + OFS_HYPER5_DICIDENT;
    else
      return NULL;
  }
  unsigned char *derefid() {
    if (_major_version == HYPER6 && _minor_version < 10)
      return buf + OFS_HYPER6_DEREFID;
    else
      return NULL;
  }

public:
  bool isAligned() {
    return _major_version >= HYPER5;
  }
  // BOCU-1辞書か？
  bool isBOCU1() {
    return _major_version >= HYPER5 && dictype() & 0x08;
  }
};

#endif

