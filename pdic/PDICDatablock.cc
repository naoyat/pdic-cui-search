// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "pdic/PDICDatablock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdic/Criteria.h"
#include "pdic/PDICDatafield.h"
#include "pdic/PDICHeader.h"
#include "pdic/PDICIndex.h"
#include "util/bocu1.h"
#include "util/charcode.h"
#include "util/types.h"
#include "util/util.h"

extern int dump_remain_count_;

PDICDatablock::PDICDatablock(byte* filemem, PDICIndex* index, int ix) {
  init(filemem, index->datablock_offset(ix), index->header);
}

PDICDatablock::PDICDatablock(byte* filemem,
                             unsigned int datablock_start_offset,
                             PDICHeader *header) {
  init(filemem, datablock_start_offset, header);
}

void PDICDatablock::init(byte* filemem,
                         unsigned int datablock_offset,
                         PDICHeader* header) {
  this->filemem = filemem;
  this->datablock_start = filemem + datablock_offset;
  this->_header = header;

  // _index = index;
  // _ix = ix;

  // _v6index = (index->header->major_version() >= HYPER6) ? true : false;
  // _isAligned = _index->header->isAligned();
  _v6index = (_header->major_version() >= HYPER6) ? true : false;
  _isAligned = _header->isAligned();

  int using_blocks_count = u16val(datablock_start);
  _is4byte = using_blocks_count & 0x8000;
  using_blocks_count &= 0x7fff;

  datablock_start += 2;
  // datablock_size = _index->datablock_block_size()*  using_blocks_count - 2;
  datablock_size = _header->block_size() * using_blocks_count - 2;
  // printf("使用ブロック数: %d (%d); %d bytes\n",
  //        using_blocks_count, is4byte, datablock_buf_size);
}

void PDICDatablock::iterate(action_proc* action, Criteria* criteria) {
  byte entry_word[1024];
  // （圧縮見出し語の伸長用）見出し語バッファ。Ver6でlword=1024なの

  for (byte* pos = datablock_start, *endpos = pos + datablock_size;
       pos < endpos; ) {
    if (dump_remain_count_ == 0) break;
    bool matched = false;

    // +0
    int field_length, compress_length, entry_word_attrib;
    byte* entry_word_compressed;
    int entry_word_compressed_size;

    if (_is4byte) {
      field_length = s32val(pos);
      pos += 4;
    } else {
      field_length = s16val(pos);
      pos += 2;
      if (field_length == 0) break;
    }

    byte* start_pos = pos, *next_pos;

    if (_isAligned) {
      compress_length = *(pos++);
      entry_word_attrib = *(pos++);
      next_pos = pos + field_length;

      entry_word_compressed = pos;
      entry_word_compressed_size =
          strlen(reinterpret_cast<char*>(entry_word_compressed));
      pos += entry_word_compressed_size + 1;
    } else {
      compress_length = *(pos++);
      next_pos = pos + field_length;

      entry_word_compressed = pos;
      entry_word_compressed_size =
          strlen(reinterpret_cast<char*>(entry_word_compressed));
      pos += entry_word_compressed_size + 1;

      entry_word_attrib = *(pos++);
    }

    int start_pos_in_filemem = static_cast<int>(start_pos - filemem);

    // byte* entry_word, int entry_word_attrib, byte* data, int datasize
    memcpy(entry_word + compress_length,
           entry_word_compressed,
           entry_word_compressed_size+1);

    PDICDatafield datafield(
        start_pos_in_filemem,
        field_length,
        entry_word,
        compress_length + entry_word_compressed_size,  // entry_word_size
        entry_word_attrib,
        // _index->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS,
        _header->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS,
        pos,  // data
        static_cast<int>(next_pos - pos),  // datasize
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
