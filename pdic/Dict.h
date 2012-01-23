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

class Dict {
 public:
  Dict(const std::string& name, byte *filemem);
  ~Dict();

 public:
  std::string info() { return name + " " + path; }
  char *prefix() { return _prefix; }
  int   make_toc();
  int   make_macdic_xml();
  void  unload_additional_files();
  bool  load_additional_files();

  std::set<int> search_in_sarray(int field, byte *needle);

  lookup_result_vec normal_lookup(byte *needle, bool exact_match);
  lookup_result_vec sarray_lookup(byte *needle);
  lookup_result_vec regexp_lookup(RE2 *re);
  lookup_result_vec full_lookup(byte *needle, RE2 *re);

  std::set<int> normal_lookup_ids(byte *needle, bool exact_match);
  std::set<int> sarray_lookup_ids(byte *needle);
  std::set<int> regexp_lookup_ids(RE2 *re);

  int word_id_for_pdic_datafield_pos(int pdic_datafield_pos);

  byte* filemem;
  std::string path, name;
  char* _prefix;

  PDICIndex *index;
  Toc*  toc;
  int   toc_length;
  byte* dict_buf[F_COUNT];
  int*  dict_suffix_array[F_COUNT], dict_suffix_array_length[F_COUNT];
  std::map<std::pair<int, int>, int> revmap;
  std::map<int, int> revmap_pdic_datafield_pos;

 public:
  static std::vector<std::string> g_dict_loadpaths_;

 private:
  lookup_result_vec ids_to_result(const std::set<int>& word_ids);
  int  rev(int field, int start_pos);
};

// match count
void reset_match_count();
void reset_render_count();
void lap_match_count();
void say_match_count();
void say_render_count();

// render
void render_result(lookup_result result, RE2 *re);

// CALLBACKS
void cb_dump_entry(PDICDatafield *datafield);
void cb_dump(PDICDatafield *datafield);
void cb_save(PDICDatafield *datafield);

void cb_estimate_buf_size(PDICDatafield *datafield);
void cb_stock_entry_words(PDICDatafield *datafield);

#endif  // PDIC_DICT_H_
