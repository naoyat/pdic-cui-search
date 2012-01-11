#include "PDICHeader.h"

#include "dump.h"

PDICHeader::PDICHeader(FILE *fp)
{
  rewind(fp);
  fread(this->buf, 1024, 1, fp);
  _version = u16val(buf + OFS_VERSION);
  _major_version = _version >> 8;
  _minor_version = _version & 0x00ff;
}

PDICHeader::~PDICHeader()
{
}

void
PDICHeader::dump()
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
