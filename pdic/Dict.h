// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDIC_DICT_H_
#define PDIC_DICT_H_

#include <stdio.h>
#include <re2/re2.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "util/types.h"

class PDICIndex;
class PDICDatafield;
class Criteria;

#define SX_TOC   ".toc"
#define SX_XML   ".xml"
#define SX_HENKAKEI_BUF  ".hbuf"
#define SX_HENKAKEI_TOC  ".htoc"

#define F_COUNT    4
#define F_ENTRY    0
#define F_JWORD    1
#define F_EXAMPLE  2
#define F_PRON     3

bool lookup_result_asc(const lookup_result& left, const lookup_result& right);
bool lookup_result_desc(const lookup_result& left, const lookup_result& right);

typedef struct {
  int pdic_datafield_pos;  // in filemem (PDICDatablock)
  int start_pos[F_COUNT];  // in .entry/.jword/.exmp/.pron
} Toc;

typedef struct {
  int henkakei_datafield_pos;
  int word_id;
} HenkakeiToc;

class Dict {
 public:
  Dict(const std::string& name, byte *filemem);
  ~Dict();

 public:
  std::string info() { return name + " " + path; }
  char *prefix() { return _prefix; }
  int   make_toc();
  int   make_macdic_xml();
  int   make_henkakei_table();
  void  unload_additional_files();
  bool  load_additional_files();

  std::set<int> search_in_sarray(int field, byte *needle);

  std::set<int> pdic_match_forward_lookup_ids(byte *needle, int flags);
  std::set<int> henkakei_lookup_ids(byte *needle, int flags);
  std::set<int> exact_lookup_ids(byte *needle, int flags);
  std::set<int> sarray_lookup_ids(byte *needle, int flags);
  std::set<int> regexp_lookup_ids(RE2 *re, int flags);

  int word_id_for_pdic_datafield_pos(int pdic_datafield_pos);

  // PDIC辞書自体が持つインデックスを用いた前方一致検索。
  lookup_result_vec pdic_match_forward_lookup(byte *needle, int flags);
  lookup_result_vec henkakei_lookup(byte *needle, int flags);
  lookup_result_vec sarray_lookup(byte *needle, int flags);
  lookup_result_vec regexp_lookup(RE2 *re, int flags);
  lookup_result_vec exact_lookup(byte *needle, int flags);
  lookup_result_vec full_lookup(byte *needle, RE2 *re, int flags);
  lookup_result_vec analyse(byte *text);

  byte* filemem;
  std::string path, name;
  char* _prefix;

  PDICIndex *index;
  Toc*  toc;
  int   toc_length;
  byte* dict_buf[F_COUNT];
  int*  dict_suffix_array[F_COUNT], dict_suffix_array_length[F_COUNT];

  // 変化形（英辞郎のみ）
  HenkakeiToc* htoc;
  int          htoc_length;
  char*        hbuf;

  std::map<std::pair<int, int>, int> revmap;
  std::map<int, int> revmap_pdic_datafield_pos;

 private:
  lookup_result_vec ids_to_result(const std::set<int>& word_ids);
  lookup_result_vec ids_to_exact_cs_result(const std::set<int>& word_ids,
                                           byte* needle);
  int  rev(int field, int start_pos);
  bool is_eijiro() {
    // printf("this is %s,\n", this->name.c_str());
    return memcmp(this->name.data(), "EIJI", 4) == 0;
  }
};

#endif  // PDIC_DICT_H_
