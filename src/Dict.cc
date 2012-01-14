#include "Dict.h"

#include <vector>
#include <string>

#include <libgen.h>
#include <strings.h>
#include <re2/re2.h>

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

bool verbose_mode = false;
bool direct_dump_mode = false;
bool full_search_mode = false;
bool ansi_coloring_mode = false;
bool more_newline_mode = false;
int render_count_limit = DEFAULT_RENDER_COUNT_LIMIT;

int _match_count, _render_count;
int _search_lap_usec, _render_lap_usec;

void reset_match_count()
{
  time_reset();
  _match_count = _render_count = 0;
}
void lap_match_count()
{
  std::pair<int,int> search_lap = time_usec();
  _search_lap_usec = search_lap.first;

  time_reset();
}
void say_match_count()
{
  std::pair<int,int> render_lap = time_usec();
  _render_lap_usec = render_lap.first;

  printf("\n// matched %d item", _match_count); if (_match_count >= 2) putchar('s');
  if (_render_count < _match_count) printf(" (rendered %d items)", _render_count);
  if (direct_dump_mode) {
    printf(", search&render:%.3fmsec", 0.001 * _search_lap_usec);
  } else {
    printf(", search:%.3fmsec", 0.001 * _search_lap_usec);
    printf(", render:%.3fmsec", 0.001 * _render_lap_usec);
  }
  printf(".\n");
}


Dict::Dict(const std::string& name, byte *filemem)//, const std::string& path)
{
  this->index = new PDICIndex(filemem);
  this->name = name;
  this->filemem = filemem;
  //this->path = path;

  this->toc = (Toc *)NULL;
  this->entry_buf = (byte *)NULL;
  this->entry_suffix_array = (int *)NULL;
  this->jword_buf = (byte *)NULL;
  this->jword_suffix_array = (int *)NULL;

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
  unloadmem(filemem); filemem = (byte*)NULL;

  delete _suffix;
  delete index;

  this->unload_additional_files();
}

// global (only for callback routines)
byte *_entry_buf = NULL; // 全ての見出し語（に'\0'を付加したデータ）を結合したもの
byte *_jword_buf = NULL; // 全ての見出し語（に'\0'を付加したデータ）を結合したもの
int _entry_buf_size = 0, _entry_buf_offset = 0;
int _jword_buf_size = 0, _jword_buf_offset = 0;
int _count = 0;
std::vector<Toc> _toc; // 各見出し語の開始位置など(entry_buf + entry_start_pos[i])

void estimate_buf_size(PDICDatafield *datafield)
{
  byte *entry_utf8 = datafield->entry_word_utf8();
  if (entry_utf8) {
    _entry_buf_size += strlen((char *)datafield->entry_word_utf8()) + 1;
  } else {
    ++_entry_buf_size;
  }

  byte *jword_utf8 = datafield->jword_utf8();
  if (jword_utf8) {
    _jword_buf_size += strlen((char *)datafield->jword_utf8()) + 1;
  } else {
    ++_jword_buf_size;
  }

  ++_count;
  if (_count & 10000 == 0) {
    printf("\r%d %d\n", _entry_buf_size, _jword_buf_size);
  }
}
void stock_entry_words(PDICDatafield *datafield)
{
  byte *entry_utf8 = datafield->entry_word_utf8();
  int entry_utf8_memsize;
  if (entry_utf8 && entry_utf8[0]) {
    entry_utf8_memsize = strlen((char *)entry_utf8) + 1;
  } else {
    entry_utf8 = (byte*)"";
    entry_utf8_memsize = 1;
  }
  if (_entry_buf_offset + entry_utf8_memsize > _entry_buf_size) {
    printf("[entry] memory over at #%d (%s)\n", (int)_toc.size(), entry_utf8);
    return;
  }

  byte *jword_utf8 = datafield->jword_utf8();
  int jword_utf8_memsize;
  if (jword_utf8 && jword_utf8[0]) {
    jword_utf8_memsize = strlen((char *)jword_utf8) + 1;
  } else {
    jword_utf8 = (byte*)"";
    jword_utf8_memsize = 1;
  }
  if (_jword_buf_offset + jword_utf8_memsize > _jword_buf_size) {
    printf("[jword] memory over at #%d (%s)\n", (int)_toc.size(), jword_utf8);
    return;
  }

  _toc.push_back((Toc){datafield->start_pos, _entry_buf_offset, _jword_buf_offset, 0, 0});

  // _entry_buf に書き足し
  memcpy(_entry_buf + _entry_buf_offset, entry_utf8, entry_utf8_memsize); // returns the position at \0
  _entry_buf_offset += entry_utf8_memsize;

  memcpy(_jword_buf + _jword_buf_offset, jword_utf8, jword_utf8_memsize); // returns the position at \0
  _jword_buf_offset += jword_utf8_memsize;
}

int
Dict::make_toc()
{
  std::cout << "インデックスを作成します..." << std::endl;
  time_reset();

  // initialize
  _toc.clear();

  _count = 0;
  _entry_buf_size = _jword_buf_size = 1;
  index->iterate_all_datablocks(&estimate_buf_size, NULL);
  //printf("サイズ: entry:%d jword:%d\n", _entry_buf_size, _jword_buf_size);
  
  _entry_buf = (byte *)malloc(_entry_buf_size);
  if (!_entry_buf) {
    printf("entry_buf is not allocated\n");
    return -1;
  }
  _entry_buf_offset = 0;
  (_entry_buf)[_entry_buf_offset++] = 0;

  _jword_buf = (byte *)malloc(_jword_buf_size);
  if (!_jword_buf) {
    printf("jword_buf is not allocated\n");
    free((void *)_entry_buf);
    return -1;
  }
  _jword_buf_offset = 0;
  (_jword_buf)[_jword_buf_offset++] = 0;

  index->iterate_all_datablocks(&stock_entry_words, NULL);

  std::pair<int,int> time = time_usec();
  printf("%d words, {%d + %d + ...}; real:%d process:%d.\n",
         (int)_toc.size(), _entry_buf_size, _jword_buf_size,
         time.first, time.second);

  time_reset();

  std::pair<int*,int> entry_sarray = make_light_suffix_array(_entry_buf, _entry_buf_size);
  int *entry_suffix_array = entry_sarray.first;
  int entry_suffix_array_length = entry_sarray.second;
  std::pair<int*,int> jword_sarray = make_light_suffix_array(_jword_buf, _jword_buf_size);
  int *jword_suffix_array = jword_sarray.first;
  int jword_suffix_array_length = jword_sarray.second;

  int toc_len = _toc.size();
  Toc *toc = (Toc *)malloc(sizeof(Toc)*toc_len);
  if (!toc) {
    printf("TOC is not allocated\n");
    return -1;
  }
  for (int i=0,c=_toc.size(); i<c; ++i) toc[i] = _toc[i];


  std::string savepath = this->suffix();
  savemem((savepath + SX_TOC).c_str(), (byte *)toc, sizeof(Toc)*_toc.size());
  savemem((savepath + SX_ENTRY).c_str(), _entry_buf, _entry_buf_size);
  savemem((savepath + SX_ENTRY_SARRAY).c_str(), (byte *)entry_suffix_array, sizeof(int)*entry_suffix_array_length);
  savemem((savepath + SX_JWORD).c_str(), _jword_buf, _jword_buf_size);
  savemem((savepath + SX_JWORD_SARRAY).c_str(), (byte *)jword_suffix_array, sizeof(int)*jword_suffix_array_length);

  free((void *)_entry_buf); _entry_buf = NULL;
  free((void *)toc);
  free((void *)entry_suffix_array);
  free((void *)jword_suffix_array);
  _toc.clear();

  this->load_additional_files();

  std::pair<int,int> time2 = time_usec();
  printf("%d/%d ; real:%d process:%d.\n", this->entry_suffix_array_length, _entry_buf_offset, time2.first, time2.second);

  return toc_len;
}


lookup_result_vec dump_result;

void dump_ej(PDICDatafield *datafield)
{
  byte *entry_word = datafield->entry_word_utf8();
  byte *jword      = datafield->jword_utf8();
  byte *example    = datafield->example_utf8();
  byte *pron       = datafield->pron_utf8();
  render_ej( std::make_pair(entry_word,jword), datafield->criteria->re2_pattern );
  if (example) printf("  // example: %s\n", example);
  if (pron) printf("  // pron: [%s]\n", pron);
  ++_match_count;
}
void dump_entry(PDICDatafield *datafield)
{
  puts((char *)datafield->entry_word_utf8());
  ++_match_count;
}
void dump_to_vector(PDICDatafield *datafield)
{
  // vector<pair<string,string> > dump_result;
  byte *entry_word = clone_cstr(datafield->entry_word_utf8());
  byte *jword = clone_cstr(datafield->jword_utf8());

  //  dump_result.push_back( std::make_pair((byte*)entry_word.c_str(), (byte*)jword.c_str()) );
  dump_result.push_back( std::make_pair(entry_word,jword) );
  ++_match_count;
}

lookup_result_vec
Dict::normal_lookup(byte *needle, bool exact_match)
{
  if (verbose_mode) {
    std::cout << "// LOOKUP \"" << needle << "\" in " << name << "..." << std::endl;
  }

  int target_charcode = index->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS;

  //Criteria *criteria;
  //if (index->header->major_version() >= HYPER6) {
  Criteria *criteria = new Criteria(needle, target_charcode, exact_match);
  byte *needle_for_index = criteria->needle_for_index ? criteria->needle_for_index : criteria->needle;
  //printf("needle for index: {%s}\n", needle_for_index);
  bsearch_result_t result = index->bsearch_in_index(needle_for_index, exact_match);
  if (verbose_mode) {
    //std::cout << "result = " << result << std::endl;
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
    //printf("lookup. from %d to %d, %d/%d...\n", from, to, to-from+1, index->_nindex);
  }

  dump_result.clear();
  
  for (int ix=from; ix<=to; ix++) {
    if (verbose_mode) {
      /*
      byte *utf8str = bocu1_to_utf8( index->entry_word(ix) );
      printf("  [%d/%d] %s\n", ix, index->_nindex, utf8str);
      free((void *)utf8str);
      */
    }
    if (ix < 0) continue;
    if (ix >= index->_nindex) break;

    PDICDatablock* datablock = new PDICDatablock(filemem, this->index, ix);
    if (direct_dump_mode) {
      datablock->iterate(&dump_ej, criteria);
    } else {
      datablock->iterate(&dump_to_vector, criteria);
    }
    delete datablock;
  }
not_found:
  ;

  return dump_result;
}


std::set<int>
Dict::search_in_sarray(byte *buf, std::map<int,int>& rev, int *sarray, int sarray_length, byte *needle)
{
  bsearch_result_t result = search(buf, sarray, sarray_length, needle, false);
  if (verbose_mode) {
    //printf("SARRAY: "); std::cout << result << std::endl;
  }

  std::set<int> matched_offsets;

  if (result.first) {
    RE2 pattern((const char *)needle);
    for (int i=result.second.first; i<=result.second.second; ++i) {
      byte *word = strhead(buf + sarray[i]);
      int offset = (int)(word - buf);
      int word_id = found(rev,offset) ? rev[offset] : -1;
      if (word_id >= 0) {
        if (direct_dump_mode) {
          render_ej( std::make_pair((byte*)entry_buf + toc[word_id].entry_start_pos,
                                    (byte*)jword_buf + toc[word_id].jword_start_pos),
                     pattern );
        //entry_word,(byte*)NULL), pattern );
        //std::cout << "- " << head << std::endl;
        } else {
          matched_offsets.insert(word_id);
        }
      }
    }
  }

  return matched_offsets;
}

lookup_result_vec
Dict::sarray_lookup(byte *needle)
{
  if (!toc || !entry_buf) {
    std::cout << "// [NOTICE] 正規表現検索には事前のインデックス作成が必要です。" << std::endl;
    std::cout << "//  → .make toc" << std::endl;
    return lookup_result_vec();
  }
  if (verbose_mode) {
    std::cout << "// SUFFIX ARRAY LOOKUP \"" << needle << "\" in " << name << "..." << std::endl;
  }

  lookup_result_vec result;

  std::set<int> matched_word_ids;
  std::set<int> entry_matches = this->search_in_entry_sarray(needle);
  matched_word_ids.insert(all(entry_matches));
  if (full_search_mode) {
    std::set<int> jword_matches = this->search_in_jword_sarray(needle);
    matched_word_ids.insert(all(jword_matches));
  }

  if (!direct_dump_mode) {
    traverse(matched_word_ids,word_id) {
      result.push_back( std::make_pair((byte*)entry_buf + toc[*word_id].entry_start_pos,
                                       (byte*)jword_buf + toc[*word_id].jword_start_pos) );
    }
    _match_count += matched_word_ids.size();
  }

  return result;
}

lookup_result_vec
Dict::regexp_lookup(const RE2& re)
{
  if (!toc || !entry_buf) {
    std::cout << "// [NOTICE] 正規表現検索には事前のインデックス作成が必要です。" << std::endl;
    std::cout << "//  → .make toc" << std::endl;
    return lookup_result_vec();
  }
  if (verbose_mode) {
    std::cout << "// REGEXP /" << re.pattern() << "/ in " << name << "..." << std::endl;
  }

  lookup_result_vec result;

  int matched_entries_count = 0;
  for (int i=0; i<toc_length; ++i) {
    const char *entry_word = (const char *)entry_buf + toc[i].entry_start_pos;
    const char *jword = (const char *)jword_buf + toc[i].jword_start_pos;
    if (RE2::PartialMatch(entry_word, re) || (full_search_mode && RE2::PartialMatch(jword, re))) {
      if (direct_dump_mode) {
        render_ej( std::make_pair((byte*)entry_word,(byte*)jword), re );
      } else {
        result.push_back( std::make_pair((byte*)entry_word, (byte*)jword) );
      }
      ++matched_entries_count;
    }
  }

  _match_count += matched_entries_count;

  return result;
}



void
Dict::unload_additional_files()
{
  if (toc) {
    unloadmem((byte *)toc);
    toc = (Toc *)NULL;
  }

  if (entry_buf) {
    unloadmem((byte *)entry_buf);
    entry_buf = NULL;
  }
  if (jword_buf) {
    unloadmem((byte *)jword_buf);
    jword_buf = NULL;
  }
  
  if (entry_suffix_array) {
    unloadmem((byte *)entry_suffix_array);
    entry_suffix_array = (int *)NULL;
  }
  if (jword_suffix_array) {
    unloadmem((byte *)jword_suffix_array);
    jword_suffix_array = (int *)NULL;
  }
}

bool
Dict::load_additional_files()
{
  unload_additional_files();

  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + this->suffix();

    if (!this->toc) {
      this->toc = (Toc *)loadmem((path + SX_TOC).c_str());
      if (this->toc) {
        this->toc_length = mem_size((byte *)this->toc) / sizeof(Toc);
        this->entry_start_rev.clear();
        this->jword_start_rev.clear();
        for (int i=0; i<this->toc_length; ++i) {
          entry_start_rev[ this->toc[i].entry_start_pos ] = i;
          jword_start_rev[ this->toc[i].jword_start_pos ] = i;
        }
      }
    }

    if (!this->entry_buf) {
      this->entry_buf = (byte *)loadmem((path + SX_ENTRY).c_str());
    }
    if (!this->jword_buf) {
      this->jword_buf = (byte *)loadmem((path + SX_JWORD).c_str());
    }

    if (!this->entry_suffix_array) {
      this->entry_suffix_array = (int *)loadmem((path + SX_ENTRY_SARRAY).c_str());
      if (this->entry_suffix_array) {
        this->entry_suffix_array_length = mem_size((byte *)this->entry_suffix_array) / sizeof(int);
      }
    }
    if (!this->jword_suffix_array) {
      this->jword_suffix_array = (int *)loadmem((path + SX_JWORD_SARRAY).c_str());
      if (this->jword_suffix_array) {
        this->jword_suffix_array_length = mem_size((byte *)this->jword_suffix_array) / sizeof(int);
      }
    }
  }

  if (verbose_mode) {
    std::cout << this->suffix() << ": loaded sarray index successfully." << std::endl;
  }

  return true;
}

void render_ej(lookup_result result, const RE2& re)
{
  if (_render_count >= render_count_limit) return;

  std::string entry_word((const char *)result.first);
  if (ansi_coloring_mode) {
    RE2::GlobalReplace(&entry_word, re, "\x1b[31m\\0\x1b[34m"); // red
    //std::cout << "\x1b[4m" << entry_word << "\x1b[24m" << std::endl; // underline
    std::cout << "\x1b[1m\x1b[34m" << entry_word << "\x1b[39m\x1b[22m" << std::endl; // bold-blue
  } else {
    std::cout << entry_word << std::endl;
  }

  if (result.second && result.second[0]) {
    std::string indent = "   ";
    std::string jword(indent + (const char *)result.second);
    //RE2::GlobalReplace(&jword, "◆", "\n◆");
    RE2::GlobalReplace(&jword, "\n", "\n"+indent);
    if (ansi_coloring_mode) {
      RE2::GlobalReplace(&jword, re, "\x1b[31m\\0\x1b[39m"); // red
    }
    std::cout << jword << std::endl;
  }

  if (more_newline_mode) std::cout << std::endl;

  ++_render_count;
}
