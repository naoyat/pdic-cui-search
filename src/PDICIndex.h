#ifndef PDICCUISEARCH_PDICINDEX_H_
#define PDICCUISEARCH_PDICINDEX_H_

#include <utility>

#include "types.h"


class PDICHeader;
class Criteria;

class PDICIndex {
public:
  explicit PDICIndex(byte *filemem);
  PDICIndex(byte *filemem, PDICHeader *header);
  ~PDICIndex();

public:
  byte *entry_word(int ix) { return index_buf + entry_word_offsets[ix]; }
  unsigned int datablock_offset(int ix);
  unsigned int datablock_block_size() { return header->block_size(); }
  bsearch_result_t bsearch_in_index(byte *needle, bool exact_match);
  bool  isBOCU1() { return _isBOCU1; }
  void  dump();
  void  iterate_datablock(int ix, action_proc *action, Criteria *criteria);
  void  iterate_all_datablocks(action_proc *action, Criteria *criteria);

  PDICHeader *header;
  int   _nindex;
  bool  _isBOCU1;
  int*  entry_word_offsets;
  int*  entry_word_lengths;
  int*  phys_ids;


private:
  int   load_index();

  byte *filemem;
  bool  header_needs_delete;
  byte *index_buf;
  int   index_size;
};

byte *string_for_index(byte *str);

#endif // PDICCUISEARCH_PDICINDEX_H_
