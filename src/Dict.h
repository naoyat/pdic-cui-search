#ifndef DICT_H
#define DICT_H

#include <string>
#include <vector>
#include <map>
#include <set>

#include <cstdio>
#include <re2/re2.h>

#include "util.h"
#include "types.h"

class PDICIndex;
class Criteria;

#define SX_PDIC           ".dic"
#define SX_TOC            ".toc"
#define SX_ENTRY          ".entry"
#define SX_ENTRY_SARRAY   ".entry.sf"
#define SX_JWORD          ".trans"
#define SX_JWORD_SARRAY   ".trans.sf"
#define SX_EXAMPLE        ".exmp"
#define SX_EXAMPLE_SARRAY ".exmp.sf"
#define SX_PRON           ".pron"
#define SX_PRON_SARRAY    ".pron.sf"

#define DEFAULT_RENDER_COUNT_LIMIT 150

//typedef std::pair<std::string,std::string> lookup_result;
//typedef std::pair<byte*,byte*> lookup_result;
typedef struct {
  byte *entry_word;
  byte *jword;
  byte *example;
  byte *pron;
} lookup_result, *lookup_result_ptr;

bool lookup_result_asc( const lookup_result_ptr& left, const lookup_result_ptr& right );
bool lookup_result_desc( const lookup_result_ptr& left, const lookup_result_ptr& right );

typedef std::vector<lookup_result*> lookup_result_vec;

typedef struct {
  int pdic_datafield_pos; // in filemem (PDICDatablock)
  int entry_start_pos;    // in .entry
  int jword_start_pos;    // in .jword
  int example_start_pos;  // in .exmp
  int pron_start_pos;     // in .pron
} Toc;

class Dict {
 public:
  byte *filemem;
  std::string path, name;
  char *_suffix;

public:
  PDICIndex *index;
  Toc  *toc;
  int   toc_length;
  byte *entry_buf, *jword_buf, *example_buf, *pron_buf;
  std::map<int,int> entry_start_rev, jword_start_rev, example_start_rev, pron_start_rev;
  int  *entry_suffix_array, entry_suffix_array_length;
  int  *jword_suffix_array, jword_suffix_array_length;
  int  *example_suffix_array, example_suffix_array_length;
  int  *pron_suffix_array, pron_suffix_array_length;
  
public:
  Dict(const std::string& name, byte *filemem);
  ~Dict();
  std::string info() { return name + " " + path; }
  char *suffix() { return _suffix; }

  int make_toc();
  void unload_additional_files();
  bool load_additional_files();

private:
  std::set<int> search_in_sarray(byte *buf, std::map<int,int>& rev, int *sarray, int sarray_length, byte *needle);
  std::set<int> search_in_entry_sarray(byte *needle) {
    return this->search_in_sarray(entry_buf, entry_start_rev, entry_suffix_array, entry_suffix_array_length, needle);
  }
  std::set<int> search_in_jword_sarray(byte *needle) {
    return this->search_in_sarray(jword_buf, jword_start_rev, jword_suffix_array, jword_suffix_array_length, needle);
  }
  std::set<int> search_in_example_sarray(byte *needle) {
    return this->search_in_sarray(example_buf, example_start_rev, example_suffix_array, example_suffix_array_length, needle);
  }
  std::set<int> search_in_pron_sarray(byte *needle) {
    return this->search_in_sarray(pron_buf, pron_start_rev, pron_suffix_array, pron_suffix_array_length, needle);
  }
public:
  lookup_result_vec normal_lookup(byte *needle, bool exact_match);
  lookup_result_vec sarray_lookup(byte *needle);
  lookup_result_vec regexp_lookup(RE2 *re);
};

// match count
void reset_match_count();
void reset_render_count();
void lap_match_count();
void say_match_count();
void say_render_count();

// render
void render_result(lookup_result *result, RE2 *re);
inline void save_result(lookup_result_vec& result_vec, lookup_result *result)
{
  if (result->entry_word && result->entry_word[0]) result->entry_word = clone_cstr(result->entry_word);
  if (result->jword && result->jword[0]) result->jword = clone_cstr(result->jword);
  if (result->example && result->example[0]) result->example = clone_cstr(result->example);
  if (result->pron && result->pron[0]) result->pron = clone_cstr(result->pron);

  result_vec.push_back((lookup_result *)clone(result, sizeof(lookup_result), true));
}

// CALLBACKS
void cb_dump_entry(PDICDatafield *datafield);
void cb_dump(PDICDatafield *datafield);
void cb_save(PDICDatafield *datafield);

void cb_estimate_buf_size(PDICDatafield *datafield);
void cb_stock_entry_words(PDICDatafield *datafield);

#endif;
