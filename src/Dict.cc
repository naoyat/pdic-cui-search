#include "Dict.h"

#include <vector>
#include <string>

#include <libgen.h>
#include <strings.h>

#include "Criteria.h"
#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatablock.h"
#include "PDICDatafield.h"
#include "bocu1.h"
#include "filemem.h"
#include "timeutil.h"
#include "util_stl.h"
#include "utf8.h"

#include "charcode.h"

#ifdef DEBUG
#include "cout.h"
#endif

extern std::vector<std::string> loadpaths;
extern bool verbose_mode;

Dict::Dict(FILE *fp, const std::string& name, const std::string& path)
{
  this->fp = fp;
  this->index = new PDICIndex(fp);
  this->name = name;
  this->path = path;

  this->entry_buf = (byte *)NULL;
  this->entry_start_pos = (int *)NULL;
  this->entry_suffix_array = (int *)NULL;

  _suffix = new char[name.size()+1];
  strcpy(_suffix, basename((char *)name.c_str()));
  int len = strlen(_suffix);
  if (strcasecmp(_suffix + len-4, ".dic") == 0) {
    _suffix[len - 4] = 0;
  }

  this->load_additional_files();
}

Dict::~Dict()
{
  fclose(fp);

  delete _suffix;
  delete index;

  unload_additional_files();
}

// global (only for callback routines)
byte *_entry_buf = NULL; // 全ての見出し語（に'\0'を付加したデータ）を結合したもの
int _entry_buf_offset = 0;
std::vector<int> _entry_start_pos; // 各見出し語の開始位置(entry_buf + entry_start_pos[i])

void stock_entry_words(PDICDatafield *datafield)
{
  byte *entry_utf8 = datafield->entry_word_utf8();
  int entry_utf8_memsize = strlen((char *)entry_utf8) + 1;
  if (_entry_buf_offset + entry_utf8_memsize > ENTRY_BUF_SIZE) {
    printf("memory over at #%d (%s)\n", (int)_entry_start_pos.size(), entry_utf8);
    return;
  }
  _entry_start_pos.push_back(_entry_buf_offset);
  memcpy(_entry_buf + _entry_buf_offset, entry_utf8, entry_utf8_memsize); // returns the position at \0
  _entry_buf_offset += entry_utf8_memsize;
}

int
Dict::make_sarray_index(int buffer_size_enough_for_whole_entries)
{
  printf("now making suffix array index...\n");
  time_reset();

  // initialize
  _entry_buf = (byte *)malloc(buffer_size_enough_for_whole_entries);
  if (!_entry_buf) {
    printf("entry_buf is not allocated\n");
    return -1;
  }
  _entry_start_pos.clear();
  _entry_buf_offset = 0;

  (_entry_buf)[_entry_buf_offset++] = 0;

  index->iterate_all_datablocks(&stock_entry_words, NULL);

  std::pair<int,int> time = time_usec();
  int entry_buf_size = _entry_buf_offset;
  printf("%d words, %d bytes; real:%d process:%d.\n", (int)_entry_start_pos.size(), entry_buf_size, time.first, time.second);

  time_reset();

  std::pair<int*,int> sarr = make_light_suffix_array(_entry_buf, entry_buf_size);
  int *suffix_array = sarr.first;
  int suffix_array_length = sarr.second;

  int *entry_start_pos = (int *)malloc(sizeof(int)*_entry_start_pos.size());
  if (!entry_start_pos) {
    printf("entry_start_pos is not allocated\n");
    return -1;
  }
  for (int i=0,c=_entry_start_pos.size(); i<c; ++i) entry_start_pos[i] = _entry_start_pos[i];

  std::string savepath = this->suffix();
  savemem((savepath + SUFFIX_ENTRY).c_str(), _entry_buf, entry_buf_size);
  savemem((savepath + SUFFIX_ENTRY_START).c_str(), (byte *)entry_start_pos, sizeof(int)*_entry_start_pos.size());
  savemem((savepath + SUFFIX_ENTRY_SARRAY).c_str(), (byte *)suffix_array, sizeof(int)*suffix_array_length);

  free((void *)_entry_buf); _entry_buf = NULL;
  free((void *)entry_start_pos);
  free((void *)suffix_array);
  _entry_start_pos.clear();

  this->load_additional_files();

  std::pair<int,int> time2 = time_usec();
  printf("%d/%d ; real:%d process:%d.\n", this->entry_suffix_array_length, _entry_buf_offset, time2.first, time2.second);

  return entry_buf_size;
}


lookup_result_vec dump_result;

void dump_to_vector(PDICDatafield *datafield)
{
  // vector<pair<string,string> > dump_result;
  byte *entry_word = clone_cstr(datafield->entry_word_utf8());
  byte *jword = clone_cstr(datafield->jword_utf8());

  //  dump_result.push_back( std::make_pair((byte*)entry_word.c_str(), (byte*)jword.c_str()) );
  dump_result.push_back( std::make_pair(entry_word, jword) );
}

lookup_result_vec
Dict::normal_lookup(byte *needle, bool exact_match)
{
  int target_charcode = index->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS;

  Criteria *criteria = new Criteria(needle, target_charcode, exact_match);

  bsearch_result_t result = index->bsearch_in_index(criteria->needle, exact_match);
  if (verbose_mode) {
    std::cout << "result = " << result << std::endl;
  }

  int from, to;
  if (result.first) {
    from = result.second.first;// - 1; if (from < 0) from = 0;
    if (bstrcmp(index->entry_word(from), criteria->needle) != 0) { --from; if (from < 0) from = 0; }
    to = result.second.second;
  } else {
    from = result.second.second - 1; if (from < 0) goto not_found;
    to = from;
  }

  if (verbose_mode) {
    printf("lookup. from %d to %d, %d/%d...\n", from, to, to-from+1, index->_nindex);
  }

  dump_result.clear();
  
  for (int ix=from; ix<=to; ix++) {
    if (verbose_mode) {
      byte *utf8str = bocu1_to_utf8( index->entry_word(ix) );
      printf("  [%d/%d] %s\n", ix, index->_nindex, utf8str);
      free((void *)utf8str);
    }
    if (ix < 0) continue;
    if (ix >= index->_nindex) break;

    PDICDatablock* datablock = new PDICDatablock(this->fp, this->index, ix);
    datablock->iterate(&dump_to_vector, criteria);
    delete datablock;
  }
not_found:
  ;

  return dump_result;
}


std::vector<int>
Dict::search_in_sarray(byte *needle)
{
  bsearch_result_t result = search(entry_buf, entry_suffix_array, entry_suffix_array_length, needle, false);
  if (verbose_mode) {
    printf("SARRAY: "); std::cout << result << std::endl;
  }

  std::set<int> matched_offsets;

  if (result.first) {
    for (int i=result.second.first; i<=result.second.second; ++i) {
      byte *head = strhead(this->entry_buf + entry_suffix_array[i]);
      int offset = (int)(head - this->entry_buf);
      matched_offsets.insert(offset);
    }
  }

  return std::vector<int>(all(matched_offsets));
}

lookup_result_vec
Dict::sarray_lookup(byte *needle)
{
  lookup_result_vec result;

  std::vector<int> matches = this->search_in_sarray(needle);
  traverse(matches, offset) {
    byte *entry_word = this->entry_buf + *offset;
    result.push_back( std::make_pair((byte*)entry_word, (byte*)"") );
  }

  return result;
}

lookup_result_vec
Dict::regexp_lookup(const RE2& pattern)
{
  if (!entry_buf || !entry_start_pos) {
    std::cout << "[NOTICE] 正規表現検索には事前のインデックス作成が必要です。" << std::endl;
    return lookup_result_vec();
  }

  lookup_result_vec result;

  int matched_entries_count = 0;
  if (verbose_mode) {
    time_reset();
  }
  for (int i=0; i<entry_start_pos_length; ++i) {
    const char *entry = (const char *)entry_buf + entry_start_pos[i];
    if (RE2::PartialMatch(entry, pattern)) {
      result.push_back( std::make_pair((byte*)entry, (byte*)"") );
      ++matched_entries_count;
    }
  }
  if (verbose_mode) {
    std::pair<int,int> time = time_usec();
    printf("%d/%d matches; real:%d process:%d.\n", matched_entries_count, entry_start_pos_length, time.first, time.second);
  }

  return result;
}



void
Dict::unload_additional_files()
{
  if (entry_buf) {
    unloadmem((byte *)entry_buf);
    entry_buf = NULL;
  }
  if (entry_start_pos) {
    unloadmem((byte *)entry_start_pos);
    entry_start_pos = (int *)NULL;
  }
  if (entry_suffix_array) {
    unloadmem((byte *)entry_suffix_array);
    entry_suffix_array = (int *)NULL;
  }
}

bool
Dict::load_additional_files()
{
  unload_additional_files();

  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + this->suffix();

    this->entry_buf = (byte *)loadmem((path + SUFFIX_ENTRY).c_str());
    if (this->entry_buf) {

      this->entry_start_pos = (int *)loadmem((path + SUFFIX_ENTRY_START).c_str());
      this->entry_start_rev.clear();
      if (this->entry_start_pos) {
        this->entry_start_pos_length = mem_size((byte *)this->entry_start_pos) / sizeof(int);
        for (int i=0; i<this->entry_start_pos_length; ++i) {
          entry_start_rev[ this->entry_start_pos[i] ] = i;
        }
      }
      this->entry_suffix_array = (int *)loadmem((path + SUFFIX_ENTRY_SARRAY).c_str());
      if (this->entry_suffix_array) {
        this->entry_suffix_array_length = mem_size((byte *)this->entry_suffix_array) / sizeof(int);
      }
      if (verbose_mode) {
        std::cout << this->suffix() << ": loaded sarray index successfully." << std::endl;
      }
      return true;
    }
  }
  return false;
}
