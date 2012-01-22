// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDICCUISEARCH_PDICDATAFIELD_H_
#define PDICCUISEARCH_PDICDATAFIELD_H_

#include <map>

#include "types.h"


class Criteria;

class PDICDatafield {
public:
  PDICDatafield(int start_pos, int field_length,
                byte *entry_word, int entry_word_size, int entry_word_attrib,
                int charcode, byte *data, int data_size,
                bool v6index, Criteria *criteria);
  ~PDICDatafield();

public:
  byte* in_utf8(int field);
  void  retain();

  int   start_pos;
  int   field_length;
  byte* entry_word;
  int   entry_word_size;
  int   entry_word_attrib;
  byte* entry_index;
  int   entry_index_size;
  int   charcode;
  byte* _entry_word_utf8;
  byte* _tabsep;
  byte* data;
  int   data_size;
  byte* _jword_utf8;
  std::map<int,byte*> _ext;
  int   _ext_flags;
  byte* _ext_start_pos;
  byte* _pron_utf8;
  byte* _example_utf8;
  bool  is_retained;
  Criteria* criteria;
  bool   v6index;

private:
  int   read_extension();
  byte* entry_word_utf8();
  byte* jword_utf8();
  byte* example_utf8();
  byte* pron_utf8();
};

#define EXT_READ     0
#define EXT_EXAMPLE  1
#define EXT_PRON     2
#define EXT_RESERVED 3
#define EXT_LINKDATA 4

#define EXT_IS_READ      (1 << EXT_READ)
#define EXT_HAS_EXAMPLE  (1 << EXT_EXAMPLE)
#define EXT_HAS_PRON     (1 << EXT_PRON)
#define EXT_HAS_RESERVED (1 << EXT_RESERVED)
#define EXT_HAS_LINKDATA (1 << EXT_LINKDATA)

#endif // PDICCUISEARCH_PDICDATAFIELD_H_
