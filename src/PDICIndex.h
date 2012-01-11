#ifndef PDIC_INDEX_H
#define PDIC_INDEX_H

class PDICHeader;
class Criteria;

#include <utility>
#include <cstdio>

#include "search.h"

class PDICIndex {
private:
  bool header_needs_delete;
  unsigned char *index_buf;
  FILE *fp;

public:
  PDICHeader *header;
  int _nindex;
  bool _isBOCU1;

public:
  //unsigned char **entry_words;
  int* entry_word_offsets;
  int* entry_word_lengths;
  int* phys_ids;
  
public:
  PDICIndex(FILE *fp);
  PDICIndex(FILE *fp, PDICHeader *header);
  ~PDICIndex();

public:
  unsigned char *entry_word(int ix) { return index_buf + entry_word_offsets[ix]; }
  unsigned int datablock_offset(int ix);
  unsigned int datablock_block_size() { return header->block_size(); }
  search_result_t bsearch_in_index(unsigned char *needle, bool exact_match) {
    return search(index_buf, entry_word_offsets, _nindex, needle, exact_match);
  }
  void dump();
  void iterate_all_datablocks(action_proc *action, Criteria *criteria);

private:
  int load_index(FILE *fp);

public:
  bool isBOCU1() { return _isBOCU1; }
};

#endif
