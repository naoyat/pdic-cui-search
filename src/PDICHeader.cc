#include "PDICHeader.h"

#include <stdio.h>

#include "dump.h"
#include "util.h"


PDICHeader::PDICHeader(byte *filemem)
{
  this->filemem = filemem;

  _version = u16val(filemem + OFS_VERSION);
  _major_version = _version >> 8;
  _minor_version = _version & 0x00ff;
}

PDICHeader::~PDICHeader()
{
}

byte* PDICHeader::headername()
{
  return filemem + OFS_HEADERNAME;
}

byte* PDICHeader::dictitle()
{
  return filemem + OFS_DICTITLE;
}

int PDICHeader::version()
{
  return _version;
}

int PDICHeader::major_version()
{
  return _major_version;
}

int PDICHeader::minor_version()
{
  return _minor_version;
}

int PDICHeader::lword()
{
  // HYPER6.10: { 1024 }
  // HYPER6.00: { 248 }
  // HYPER4, HYPER5: { 248 }
  return s16val(filemem + OFS_LWORD);
}

int PDICHeader::ljapa()
{
  // HYPER6.10: { 262144 } 未参照
  // HYPER6.00: { 10000 }
  // HYPER4, HYPER5: { 3000 }
  return s16val(filemem + OFS_LJAPA);
}

int PDICHeader::block_size()
{
  // HYPER6: { 1024 }
  // HYPER4, HYPER5: { 256 }
  // NEWDIC2: { 64, 128, 256, 512 }*16
  // NEWDIC1: { 1024, 2048, 4096, 8192 }
  if (_major_version >= HYPER4)
    return u16val(filemem + OFS_BLOCK_SIZE);
  else if (_major_version >= NEWDIC2)
    return 16 * s16val(filemem + OFS_BLOCK_SIZE);
  else // NEWDIC
    return s16val(filemem + OFS_BLOCK_SIZE);
}

int PDICHeader::index_block()
{
  if (_major_version >= HYPER4)
    return s16val(filemem + OFS_INDEX_BLOCK);
  else // NEWDIC, NEWDIC2
    return 2048 * s16val(filemem + OFS_INDEX_BLOCK);
}

int PDICHeader::header_size()
{
  // HYPER6: { 1024 }
  // HYPER4, HYPER5: { 256 }
  return s16val(filemem + OFS_HEADER_SIZE); // 256, 1024(6〜)
}

int PDICHeader::index_size()
{
  if (_major_version >= HYPER4)
    return index_block() * block_size();
  else // NEWDIC, NEWDIC2
    return s16val(filemem + OFS_INDEX_SIZE); // { 2048, 10240, 20480, 40960 }
}

int PDICHeader::empty_block()
{
  // -1 if not exists
  if (_major_version >= HYPER5)
    return s32val(filemem + OFS_HYPER5_EMPTY_BLOCK2);
  else if (_major_version >= HYPER4)
    return s32val(filemem + OFS_HYPER4_EMPTY_BLOCK2);
  else // NEWDIC, NEWDIC2
    return s16val(filemem + OFS_EMPTY_BLOCK); // [0-4095]
}

int PDICHeader::nindex()
{
  if (_major_version >= HYPER5)
    return u32val(filemem + OFS_HYPER5_NINDEX2);
  else if (_major_version >= HYPER4)
    return u32val(filemem + OFS_HYPER4_NINDEX2);
  else // NEWDIC, NEWDIC2
    return s16val(filemem + OFS_NINDEX); // <= 4096
}

int PDICHeader::nblock()
{
  if (_major_version >= HYPER5)
    return u32val(filemem + OFS_HYPER5_NBLOCK2);
  else if (_major_version >= HYPER4)
    return u32val(filemem + OFS_HYPER4_NBLOCK2);
  else // NEWDIC, NEWDIC2
    return s16val(filemem + OFS_NBLOCK);
}

unsigned int PDICHeader::nword()
{
  return u32val(filemem + OFS_NWORD);
}

int PDICHeader::dicorder()
{
  // { 0[, 1, 2, 3] }
  if (_major_version >= NEWDIC2)
    return filemem[OFS_DICORDER];
  else
    return 0;
}

int PDICHeader::dictype()
{
  // HYPER6:  { 0x01 | 0x08 |[0x10]|[0x20]| 0x40 |[0x80]}
  // HYPER5:  { 0x01 | 0x08 | 0x10 |[0x20]| 0x40 | 0x80 }
  // HYPER4:  { 0x01 |      |[0x10]| 0x20 | 0x40 | 0x80 }
  // NEWDIC2: { 0x01 |      | 0x10 | 0x20 }
  if (_major_version >= NEWDIC2)
    return filemem[OFS_DICTYPE];
  else
    return 0;
}

int PDICHeader::attrlen()
{
  // { 1 }
  return filemem[OFS_ATTRLEN];
}

int PDICHeader::olenumber()
{
  if (_major_version >= HYPER5)
    return s32val(filemem + OFS_HYPER5_OLENUMBER);
  else if (_major_version >= NEWDIC2)
    return s32val(filemem + OFS_NEWDIC2_OLENUMBER);
  else
    return 0;
}

int PDICHeader::os()
{
  // HYPER6: { 0x20="BOCU-1 encoding" }
  // HYPER5: { 0, 1, 2, 3, 4, 0x10 }
  // HYPER4: { 0 }
  // NEWDIC2: { 0[, 1, 2] }
  if (_major_version >= HYPER5)
    return filemem[OFS_HYPER5_OS];
  else if (_major_version >= NEWDIC2)
    return filemem[OFS_NEWDIC2_OS];
  else
    return 0;
}

int PDICHeader::lid_word()
{
  return _major_version == NEWDIC2 ? u16val(filemem + OFS_LID_WORD) : 0;
}

int PDICHeader::lid_japa()
{
  return _major_version == NEWDIC2 ? u16val(filemem + OFS_LID_JAPA) : 0;
}

int PDICHeader::lid_exp()
{
  return _major_version == NEWDIC2 ? u16val(filemem + OFS_LID_EXP) : 0;
}

int PDICHeader::lid_pron()
{
  return _major_version == NEWDIC2 ? u16val(filemem + OFS_LID_PRON) : 0;
}

int PDICHeader::lid_other()
{
  return _major_version == NEWDIC2 ? u16val(filemem + OFS_LID_OTHER) : 0;
}

int PDICHeader::index_blkbit()
{
  // 0:16bit, 1:32bit
  if (_major_version >= HYPER5)
    return filemem[OFS_HYPER5_INDEX_BLKBIT];
  else if (_major_version == HYPER4)
    return filemem[OFS_HYPER4_INDEX_BLKBIT];
  else
    return 0;
}

unsigned int PDICHeader::extheader()
{
  if (_major_version >= HYPER5)
    return u32val(filemem + OFS_HYPER5_EXTHEADER);
  else if (_major_version == HYPER4)
    return u32val(filemem + OFS_HYPER4_EXTHEADER);
  else
    return 0;
}

byte* PDICHeader::cypt()
{
  if (_major_version >= HYPER6)
    return filemem + OFS_HYPER6_CYPT;
  else
    return NULL;
}

unsigned int PDICHeader::update_count()
{
  if (_major_version >= HYPER4)
    return u32val(filemem + OFS_UPDATE_COUNT);
  else
    return 0;
}

byte* PDICHeader::dicident()
{
  if (_major_version >= HYPER5)
    return filemem + OFS_HYPER5_DICIDENT;
  else
    return NULL;
}

byte* PDICHeader::derefid()
{
  if (_major_version == HYPER6 && _minor_version < 10)
    return filemem + OFS_HYPER6_DEREFID;
  else
    return NULL;
}

bool PDICHeader::isAligned()
{
  return _major_version >= HYPER5;
}

// BOCU-1辞書か？
bool PDICHeader::isBOCU1()
{
  return _major_version >= HYPER5 && dictype() & 0x08;
}

void PDICHeader::dump()
{
  //  ::dump(this->buf, 1024);
  printf("    headername = \"%s\"\n", headername());
  printf("      dictitle = \"%s\"\n", dictitle());
  printf("       version = %x\n", _version);
  printf("         lword = %d\n", lword());
  printf("         ljapa = %d\n", ljapa());
  printf("    block_size = %d\n", block_size());
  printf("   index_block = %d\n", index_block());
  printf("   header_size = %d\n", block_size());
  printf(" index_size(*) = %d\n", index_size());
  printf("empty_block(*) = %d\n", empty_block());
  printf("     nindex(*) = %d\n", nindex());
  printf("     nblock(*) = %d\n", nblock());
  printf("         nword = %d\n", nword());
  printf("      dicorder = %d\n", dicorder());
  printf("       dictype = %02x\n", dictype());
  printf("       attrlen = %d\n", attrlen());
  printf("            os = %02x\n", os());
  printf("     olenumber = %d\n", olenumber());
  // dummy_lid
  printf("  index_blkbit = %d\n", index_blkbit());
  // dummy0
  printf("     extheader = %d\n", extheader());
  // empty_block2, nindex2, nblock2
  printf("          cypt = "); inline_dump(cypt(), 8); printf("\n");
  printf("  update_count = %d\n", update_count());
  // dummy00 4
  printf("      dicident = "); inline_dump(dicident(), 8); printf("\n");
  // derefid 8
  // dummy 24
  printf("\n");
}
