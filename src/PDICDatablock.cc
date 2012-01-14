#include "PDICDatablock.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatafield.h"
#include "Criteria.h"

#include "util.h"
#include "bocu1.h"
#include "charcode.h"
#include "types.h"

PDICDatablock::PDICDatablock(byte *filemem, PDICIndex *index, int ix)
{
  this->filemem = filemem;
  this->datablock_start = filemem + index->datablock_offset(ix);

  _index = index;
  _ix = ix;

  _v6index = (index->header->major_version() >= HYPER6) ? true : false;
  _isAligned = _index->header->isAligned();

  int using_blocks_count = u16val(datablock_start);
  _is4byte = using_blocks_count & 0x8000; using_blocks_count &= 0x7fff;

  datablock_start += 2;
  datablock_size = _index->datablock_block_size() * using_blocks_count - 2;
  //printf("使用ブロック数: %d (%d); %d bytes\n", using_blocks_count, is4byte, datablock_buf_size);
}

void
PDICDatablock::iterate(action_proc *action, Criteria *criteria)
{
  byte entry_word[1024]; // （圧縮見出し語の伸長用）見出し語バッファ。Ver6でlword=1024なの

  for (byte *pos=datablock_start,*endpos=pos+datablock_size; pos<endpos; ) {
    bool matched = false;

    // +0
    int field_length, compress_length, entry_word_attrib;
    byte *entry_word_compressed;
    int entry_word_compressed_size;

    if (_is4byte) {
      field_length = s32val(pos); pos += 4;
    } else {
      field_length = s16val(pos); pos += 2;
      if (field_length == 0) break;
    }

    byte *start_pos = pos, *next_pos;

    if (_isAligned) {
      compress_length = *(pos++);
      entry_word_attrib = *(pos++);
      next_pos = pos + field_length;

      entry_word_compressed = pos;
      entry_word_compressed_size = strlen((char *)entry_word_compressed);
      pos += entry_word_compressed_size + 1;
    } else {
      compress_length = *(pos++);
      next_pos = pos + field_length;

      entry_word_compressed = pos;
      entry_word_compressed_size = strlen((char *)entry_word_compressed);
      pos += entry_word_compressed_size + 1;

      entry_word_attrib = *(pos++);
    }

    // byte* entry_word, int entry_word_attrib, byte* data, int datasize
    memcpy(entry_word + compress_length, entry_word_compressed, entry_word_compressed_size+1);

    PDICDatafield datafield((int)(start_pos - filemem),
                            field_length,
                            entry_word,
                            compress_length + entry_word_compressed_size, // entry_word_size
                            entry_word_attrib,
                            _index->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS,
                            pos, // data
                            (int)(next_pos - pos), // datasize
                            _v6index,
                            criteria);

    if (criteria) {
      matched = criteria->match(&datafield);
    } else {
      matched = true;
    }

    if (matched) {
      (*action)(&datafield);
    }

    if (_is4byte) break;
    pos = next_pos;
  }
}
