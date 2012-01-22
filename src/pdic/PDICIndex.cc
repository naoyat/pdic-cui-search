// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pdic/PDICDatablock.h"
#include "pdic/PDICHeader.h"
#include "pdic/PDICIndex.h"
#include "util/bocu1.h"
#include "util/dump.h"
#include "util/search.h"
#include "util/utf8.h"
#include "util/util.h"

PDICIndex::PDICIndex(byte* filemem) {
  this->filemem = filemem;
  this->header = new PDICHeader(filemem);
  header_needs_delete = true;
  load_index();
}

PDICIndex::PDICIndex(byte* filemem, PDICHeader* header) {
  this->filemem = filemem;
  this->header = header;
  header_needs_delete = false;
  load_index();
}

PDICIndex::~PDICIndex() {
  // if (index_buf) delete index_buf;

  if (header_needs_delete) delete header;
  delete entry_word_offsets;
  delete entry_word_lengths;
  delete phys_ids;
}

int PDICIndex::load_index() {
  index_buf = filemem + header->header_size() + header->extheader();
  index_size = header->index_size();

  _nindex = header->nindex();
  _isBOCU1 = header->isBOCU1();
  int index_blkbit = header->index_blkbit();
  // ? 4 : 2; // 0->2(16bit), 1->4(32bit)

  entry_word_offsets = new int[_nindex];
  entry_word_lengths = new int[_nindex];
  phys_ids = new int[_nindex];

  int actual_nindex = 0;
  for (int ix = 0, ofs = 0; ix < _nindex && ofs < index_size; ++ix) {
    int phys_id;
    if (index_blkbit == 0) {
      phys_id = s16val(index_buf + ofs);
      ofs += 2;
    } else {
      phys_id = s32val(index_buf + ofs);
      ofs += 4;
    }

    if (ofs >= index_size) break;

    byte* entry_word = index_buf + ofs;  // cstr
    int entry_word_length = strlen(reinterpret_cast<char*>(entry_word));

    if (phys_ids[ix] == 0 && entry_word_length == 0) break;

    phys_ids[ix] = phys_id;
    entry_word_offsets[ix] = ofs;  // entry_word;
    entry_word_lengths[ix] = entry_word_length;

    ofs += entry_word_length + 1;
    ++actual_nindex;
  }

  return actual_nindex;
}

unsigned int PDICIndex::datablock_offset(int ix) {
  if (ix < 0 || _nindex <= ix) return 0;

  unsigned int offset =
      header->header_size() + header->extheader() + header->index_size()
      + header->block_size()*phys_ids[ix];

  return offset;
}

bsearch_result_t PDICIndex::bsearch_in_index(byte* needle, bool exact_match) {
  return search(index_buf, entry_word_offsets, _nindex, needle, exact_match);
}

void PDICIndex::dump() {
  for (int ix = 0; ix < _nindex; ++ix) {
    printf("%04d +%d: ", 1+ix, phys_ids[ix]);

    if (_isBOCU1) {
      bocu1_dump_in_utf8(entry_word(ix), entry_word_lengths[ix]);
    // bocu1_check(entry_words[ix]);
    } else {
      printf("%*s", entry_word_lengths[ix], entry_word(ix));
    }

    printf("\n");
  }
}

void PDICIndex::iterate_datablock(int ix,
                                  action_proc* action,
                                  Criteria* criteria) {
  PDICDatablock* datablock = new PDICDatablock(filemem, this, ix);
  datablock->iterate(action, criteria);
  delete datablock;
}

void PDICIndex::iterate_all_datablocks(action_proc* action,
                                       Criteria* criteria) {
  for (int ix = 0; ix < _nindex; ++ix)
    this->iterate_datablock(ix, action, criteria);
}

//
// PDIC6 utility
//
unichar katakana_map[0x60] = {
  0,      0x30A2, 0,      0x30A4, 0,      0x30A6, 0,      0x30A8,
  0,      0x30AA, 0,      0,      0x30AB, 0,      0x30AD, 0,
  0x30AF, 0,      0x30B1, 0,      0x30B3, 0,      0x30B5, 0,
  0x30B7, 0,      0x30B9, 0,      0x30BB, 0,      0x30BD, 0,
  0x30BF, 0,      0x30C1, 0x30C4, 0,      0x30C4, 0,      0x30C6,
  0,      0x30C8, 0,      0,      0,      0,      0,      0,
  0x30CF, 0x30CF, 0,      0x30D2, 0x30D2, 0,      0x30D5, 0x30D5,
  0,      0x30D8, 0x30D8, 0,      0x30DB, 0x30DB, 0,      0,
  0,      0,      0,      0x30E4, 0,      0x30E6, 0,      0x30E8,
  0,      0,      0,      0,      0,      0,      0x30EF, 0,
  0,      0,      0,      0,      0x30A6, 0x30AB, 0x30B1, 0x30EF,
  0x30F0, 0x30F1, 0x30F2, 0,      0,      0,      0,      0 };

byte* string_for_index(byte* src_str) {
  int src_cp_length;
  unichar* src_cp = decode_utf8(src_str,
                                strlen(reinterpret_cast<char*>(src_str)),
                                &src_cp_length);

  int dest_cp_length = 0;
  unichar* dest_cp = static_cast<unichar*>(
      malloc(sizeof(unichar)*(src_cp_length + 1)));
  for (int i = 0; i < src_cp_length; ++i) {
    int ch_s = src_cp[i], ch_d;

    if (isupper(ch_s)) {
      ch_d = tolower(ch_s);
    } else if (ch_s == '-') {
      ch_d = ' ';
    } else if (ch_s == '\'') {
      continue;  // skip quote
    } else if (ch_s == 0x3001) {
      ch_d = ',';  // 、
    } else if (ch_s == 0x3002) {
      ch_d = '.';  // 。
    } else if (ch_s == 0x301c) {
      ch_d = '~';  // 〜
    } else if (0x30a0 <= ch_s && ch_s <= 0x30ff) {
      int ch = katakana_map[ch_s - 0x30a0];
      ch_d = ch ? ch : ch_s;
    } else if (0xff00 <= ch_s && ch_s <= 0xff9f) {
      ch_d = ch_s - 0xff00 + 0x20;
    } else {
      ch_d = ch_s;
    }

    dest_cp[dest_cp_length++] = ch_d;
  }
  dest_cp[dest_cp_length] = 0;
  free(static_cast<void*>(src_cp));

  int dest_size;
  byte* dest_str = encode_utf8(dest_cp, dest_cp_length, &dest_size);

  free(static_cast<void*>(dest_cp));

  return dest_str;
}
