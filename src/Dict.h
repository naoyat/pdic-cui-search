#ifndef PDICCUISEARCH_DICT_H_
#define PDICCUISEARCH_DICT_H_

#include <stdio.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include <re2/re2.h>

#include "types.h"


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

#define DEFAULT_RENDER_COUNT_LIMIT 150

bool lookup_result_asc( const lookup_result& left, const lookup_result& right );
bool lookup_result_desc( const lookup_result& left, const lookup_result& right );

typedef struct {
  int pdic_datafield_pos; // in filemem (PDICDatablock)
  int start_pos[F_COUNT]; // in .entry/.jword/.exmp/.pron
} Toc;

class Dict {

public:
  byte *filemem;
  std::string path, name;
  char *_prefix;

public:
  PDICIndex *index;
  Toc  *toc;
  int   toc_length;
  byte *dict_buf[F_COUNT];
  int  *dict_suffix_array[F_COUNT], dict_suffix_array_length[F_COUNT];
  std::map<std::pair<int,int>,int> revmap;
  std::map<int,int> revmap_pdic_datafield_pos;

public:
  Dict(const std::string& name, byte *filemem);
  ~Dict();
  std::string info() { return name + " " + path; }
  char *prefix() { return _prefix; }

  int make_toc();
  int make_macdic_xml();
  void unload_additional_files();
  bool load_additional_files();

public:
  std::set<int> search_in_sarray(int field, byte *needle);

  lookup_result_vec normal_lookup(byte *needle, bool exact_match) {
    std::set<int> matched_word_ids = normal_lookup_ids(needle, exact_match);
    return ids_to_result(matched_word_ids);
  }
  lookup_result_vec sarray_lookup(byte *needle) {
    std::set<int> matched_word_ids = sarray_lookup_ids(needle);
    return ids_to_result(matched_word_ids);
  }
  lookup_result_vec regexp_lookup(RE2 *re) {
    std::set<int> matched_word_ids = regexp_lookup_ids(re);
    return ids_to_result(matched_word_ids);
  }
  lookup_result_vec full_lookup(byte *needle, RE2 *re) {
    std::set<int> matched_word_ids;

    std::set<int> matched_word_ids_normal = normal_lookup_ids(needle, false);
    matched_word_ids.insert(matched_word_ids_normal.begin(), matched_word_ids_normal.end());

    std::set<int> matched_word_ids_sarray = sarray_lookup_ids(needle);
    matched_word_ids.insert(matched_word_ids_sarray.begin(), matched_word_ids_sarray.end());

    std::set<int> matched_word_ids_regexp = regexp_lookup_ids(re);
    matched_word_ids.insert(matched_word_ids_regexp.begin(), matched_word_ids_regexp.end());

    return ids_to_result(matched_word_ids);
  }

  std::set<int> normal_lookup_ids(byte *needle, bool exact_match);
  std::set<int> sarray_lookup_ids(byte *needle);
  std::set<int> regexp_lookup_ids(RE2 *re);

  int word_id_for_pdic_datafield_pos(int pdic_datafield_pos);

private:
  lookup_result_vec ids_to_result(const std::set<int>& word_ids);
  int rev(int field, int start_pos);
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

#endif // PDICCUISEARCH_DICT_H_
