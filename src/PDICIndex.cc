#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatablock.h"
//#include "Criteria.h"

#include "dump.h"
#include "bocu1.h"

PDICIndex::PDICIndex(FILE *fp)
{
  this->header = new PDICHeader(fp);
  this->fp = fp;
  header_needs_delete = true;
  load_index(fp);
}
PDICIndex::PDICIndex(FILE *fp, PDICHeader *header)
{
  this->header = header;
  this->fp = fp;
  header_needs_delete = false;
  load_index(fp);
}
PDICIndex::~PDICIndex()
{
  if (index_buf) delete index_buf;

  if (header_needs_delete) delete header;
  delete entry_word_offsets;
  delete entry_word_lengths;
  delete phys_ids;
}

int
PDICIndex::load_index(FILE *fp)
{
  int index_offset = header->header_size() + header->extheader();
  fseek(fp, index_offset, SEEK_SET);
  
  int index_size = header->index_size();
  index_buf = new unsigned char[index_size]; //(unsigned char *)malloc(index_size);
  if (index_buf == NULL) return 0;

  _nindex = header->nindex();
  _isBOCU1 = header->isBOCU1();
  int index_blkbit = header->index_blkbit();// ? 4 : 2; // 0->2(16bit), 1->4(32bit)

  size_t size = fread(index_buf, index_size, 1, fp);
  if (size != 1) return 0;

  entry_word_offsets = new int[_nindex];
  entry_word_lengths = new int[_nindex];
  phys_ids = new int[_nindex];
  //entry_words = (unsigned char **)malloc(sizeof(unsigned char *)*nindex);
  //phys_ids = (int *)malloc(sizeof(int)*nindex);

  int actual_nindex = 0;
  for (int ix=0,ofs=0; ix<_nindex && ofs<index_size; ++ix) {
    //nindex; int ofs=0; ofs<index_size; ) {
    int phys_id;
    if (index_blkbit == 0) {
      phys_id = s16val(index_buf + ofs); ofs += 2;
    } else {
      phys_id = s32val(index_buf + ofs); ofs += 4;
    }
    
    if (ofs >= index_size) break;

    unsigned char *entry_word = index_buf + ofs; // cstr
    int entry_word_length = strlen((char *)entry_word);

    if (phys_ids[ix] == 0 && entry_word_length == 0) break;

    phys_ids[ix] = phys_id;
    entry_word_offsets[ix] = ofs; // entry_word;
    entry_word_lengths[ix] = entry_word_length;

    ofs += entry_word_length + 1;
    ++actual_nindex;
  }

  return actual_nindex;
}

unsigned int
PDICIndex::datablock_offset(int ix)
{
  if (ix < 0 || _nindex <= ix) return 0;

  unsigned int offset = header->header_size() + header->extheader() + header->index_size()
      + header->block_size()*phys_ids[ix];

  return offset;
}

void
PDICIndex::dump()
{
  for (int ix=0; ix<_nindex; ++ix) {
    printf("%04d +%d: ", 1+ix, phys_ids[ix]);

    if (_isBOCU1) {
      bocu1_dump_in_utf8(entry_word(ix), entry_word_lengths[ix]);
    // bocu1_check(entry_words[ix]);
    } else {
      printf("%*s", entry_word_lengths[ix], entry_word(ix));
    }
    newline();
  }
}

void
PDICIndex::iterate_all_datablocks(action_proc *action, Criteria *criteria)
{
  for (int ix=0; ix<_nindex; ++ix) {
    /*
    printf("%04d +%d: ", 1+ix, phys_ids[ix]);

    if (_isBOCU1) {
      bocu1_dump_in_utf8(entry_words[ix], entry_word_lengths[ix]);
    } else {
      printf("%*s", entry_word_lengths[ix], entry_words[ix]);
    }
    newline();
    */

    PDICDatablock *datablock = new PDICDatablock(fp, this, ix);
    datablock->iterate(action, criteria);
    delete datablock;
  }
}
