#include "Dict.h"

#include <iostream>
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
#include "ansi_color.h"

#ifdef DEBUG
#include "cout.h"
#endif

bool lookup_result_asc( const lookup_result_ptr& left, const lookup_result_ptr& right)
{
  return bstrcmp(left->entry_word, right->entry_word) < 0;
}
bool lookup_result_desc( const lookup_result_ptr& left, const lookup_result_ptr& right )
{
  return bstrcmp(left->entry_word, right->entry_word) > 0;
}

extern std::vector<std::string> loadpaths;

bool separator_mode = false;
bool verbose_mode = false;
bool direct_dump_mode = false;
bool full_search_mode = false;
bool ansi_coloring_mode = false;
bool more_newline_mode = false;
int render_count_limit = DEFAULT_RENDER_COUNT_LIMIT;

int match_count, render_count;
int _search_lap_usec, _render_lap_usec;

lookup_result_vec _result_vec;

void reset_match_count()
{
  time_reset();
  match_count = 0;
}
void reset_render_count()
{
  time_reset();
  render_count = 0;
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

  printf(ANSI_FGCOLOR_GREEN "// 結果%d件", match_count); // if (_match_count >= 2) putchar('s');
  if (render_count < match_count) printf(" (うち%d件表示)", render_count);
  if (direct_dump_mode) {
    printf(", 検索+表示:%.3fミリ秒", 0.001 * _search_lap_usec);
  } else {
    printf(", 検索:%.3fミリ秒", 0.001 * _search_lap_usec);
    printf(", 表示:%.3fミリ秒", 0.001 * _render_lap_usec);
  }
  printf(".\n" ANSI_FGCOLOR_DEFAULT);
}
void say_render_count()
{
  std::pair<int,int> render_lap = time_usec();
  _render_lap_usec = render_lap.first;

  printf(ANSI_FGCOLOR_GREEN "// 結果%d件", match_count); // if (_match_count >= 2) putchar('s');
  if (render_count < match_count) printf(" (うち%d件表示)", render_count);
  printf(", 表示:%.3fミリ秒", 0.001 * _render_lap_usec);
  printf(".\n" ANSI_FGCOLOR_DEFAULT);
}


Dict::Dict(const std::string& name, byte *filemem)//, const std::string& path)
{
  index = new PDICIndex(filemem);
  this->name = name;
  this->filemem = filemem;

  toc = (Toc *)NULL;
  entry_buf = jword_buf = example_buf = pron_buf = (byte *)NULL;
  entry_suffix_array = jword_suffix_array = example_suffix_array = pron_suffix_array = (int *)NULL;

  _suffix = new char[name.size()+1];
  strcpy(_suffix, basename((char *)name.c_str()));
  int len = strlen(_suffix);
  if (strcasecmp(_suffix + len-4, ".dic") == 0) {
    _suffix[len - 4] = 0;
  }

  load_additional_files();
}

Dict::~Dict()
{
  unloadmem(filemem); filemem = (byte*)NULL;

  delete _suffix;
  delete index;

  unload_additional_files();
}

// global (only for callback routines)
byte *_entry_buf = NULL; // 全ての見出し語（に'\0'を付加したデータ）を結合したもの
byte *_jword_buf = NULL; // 全ての訳語（〃
byte *_example_buf = NULL; // 全ての用例（〃
byte *_pron_buf = NULL; // 全ての発音記号（〃
int _entry_buf_size = 0, _entry_buf_offset = 0;
int _jword_buf_size = 0, _jword_buf_offset = 0;
int _example_buf_size = 0, _example_buf_offset = 0;
int _pron_buf_size = 0, _pron_buf_offset = 0;
int _count = 0;
std::vector<Toc> _toc; // 各見出し語の開始位置など(entry_buf + entry_start_pos[i])

void cb_estimate_buf_size(PDICDatafield *datafield)
{
  byte *entry_utf8 = datafield->entry_word_utf8();
  if (entry_utf8 && entry_utf8[0]) {
    _entry_buf_size += strlen((char *)datafield->entry_word_utf8()) + 1;
  }

  byte *jword_utf8 = datafield->jword_utf8();
  if (jword_utf8 && jword_utf8[0]) {
    _jword_buf_size += strlen((char *)datafield->jword_utf8()) + 1;
  }

  byte *example_utf8 = datafield->example_utf8();
  if (example_utf8 && example_utf8[0]) {
    _example_buf_size += strlen((char *)datafield->example_utf8()) + 1;
  }

  byte *pron_utf8 = datafield->pron_utf8();
  if (pron_utf8 && pron_utf8[0]) {
    _pron_buf_size += strlen((char *)datafield->pron_utf8()) + 1;
  }

  ++_count;

  if (_count & 1000 == 0) {
    printf("%9d %9d %9d %9d\r", _entry_buf_size, _jword_buf_size, _example_buf_size, _pron_buf_size);
  }
}

void cb_stock_entry_words(PDICDatafield *datafield)
{
  Toc toc;

  toc.pdic_datafield_pos = datafield->start_pos;

  byte *entry_utf8 = datafield->entry_word_utf8();
  if (entry_utf8 && entry_utf8[0]) {
    int entry_utf8_memsize = strlen((char *)entry_utf8) + 1;
    memcpy(_entry_buf + _entry_buf_offset, entry_utf8, entry_utf8_memsize); // returns the position at \0
    toc.entry_start_pos = _entry_buf_offset;
    _entry_buf_offset += entry_utf8_memsize;
  } else {
    toc.entry_start_pos = 0;
  }

  byte *jword_utf8 = datafield->jword_utf8();
  if (jword_utf8 && jword_utf8[0]) {
    int jword_utf8_memsize = strlen((char *)jword_utf8) + 1;
    memcpy(_jword_buf + _jword_buf_offset, jword_utf8, jword_utf8_memsize); // returns the position at \0
    toc.jword_start_pos = _jword_buf_offset;
    _jword_buf_offset += jword_utf8_memsize;
  } else {
    toc.jword_start_pos = 0;
  }

  byte *example_utf8 = datafield->example_utf8();
  if (example_utf8 && example_utf8[0]) {
    int example_utf8_memsize = strlen((char *)example_utf8) + 1;
    memcpy(_example_buf + _example_buf_offset, example_utf8, example_utf8_memsize); // returns the position at \0
    toc.example_start_pos = _example_buf_offset;
    _example_buf_offset += example_utf8_memsize;
  } else {
    toc.example_start_pos = 0;
  }

  byte *pron_utf8 = datafield->pron_utf8();
  if (pron_utf8 && pron_utf8[0]) {
    int pron_utf8_memsize = strlen((char *)pron_utf8) + 1;
    memcpy(_pron_buf + _pron_buf_offset, pron_utf8, pron_utf8_memsize); // returns the position at \0
    toc.pron_start_pos = _pron_buf_offset;
    _pron_buf_offset += pron_utf8_memsize;
  } else {
    toc.pron_start_pos = 0;
  }

  _toc.push_back(toc);
}

int
Dict::make_toc()
{
  std::cout << name << ": インデックスを作成します..." << std::endl;
  time_reset();

  // initialize

  //
  // PASS1: 先に必要なメモリ量を計算してしまう
  //
  _count = 0;
  _entry_buf_size = _jword_buf_size = _example_buf_size = _pron_buf_size = 1;

  index->iterate_all_datablocks(&cb_estimate_buf_size, NULL);
  //printf("サイズ: entry:%d jword:%d\n", _entry_buf_size, _jword_buf_size);

  _entry_buf = (byte *)malloc(_entry_buf_size);
  if (!_entry_buf) {
    printf("entry_buf is not allocated\n");
    return -1;
  }

  _jword_buf = (byte *)malloc(_jword_buf_size);
  if (!_jword_buf) {
    printf("jword_buf is not allocated\n");
    free((void *)_entry_buf);
    return -1;
  }

  _example_buf = (byte *)malloc(_example_buf_size);
  if (!_example_buf) {
    printf("example_buf is not allocated\n");
    free((void *)_entry_buf);
    free((void *)_jword_buf);
    return -1;
  }

  _pron_buf = (byte *)malloc(_pron_buf_size);
  if (!_pron_buf) {
    printf("pron_buf is not allocated\n");
    free((void *)_entry_buf);
    free((void *)_jword_buf);
    free((void *)_example_buf);
    return -1;
  }

  //
  // PASS2: 全てのデータブロックから全ての単語データを抽出
  //
  _toc.clear();

  _entry_buf[0] = _jword_buf[0] = _example_buf[0] = _pron_buf[0] = 0;
  _entry_buf_offset = _jword_buf_offset = _example_buf_offset = _pron_buf_offset = 1;

  index->iterate_all_datablocks(&cb_stock_entry_words, NULL);

  std::pair<int,int> time = time_usec();
  printf("データ抽出: %d words, {%d + %d + %d + %d}; real:%.3fmsec process:%.3fmsec\n",
         (int)_toc.size(), _entry_buf_size, _jword_buf_size, _example_buf_size, _pron_buf_size,
         0.001*time.first, 0.001*time.second);

  time_reset();

  //
  // 保存
  //
  std::string savepath = this->suffix();

  // TOC
  int toc_len = _toc.size();
  Toc *toc = (Toc *)malloc(sizeof(Toc)*toc_len);
  if (!toc) {
    printf("TOC is not allocated\n");
    return -1;
  }
  for (int i=0,c=_toc.size(); i<c; ++i) toc[i] = _toc[i];

  savemem((savepath + SX_TOC).c_str(), (byte *)toc, sizeof(Toc)*_toc.size());
  free((void *)toc);
  _toc.clear();

  // entry sarray
  std::pair<int*,int> entry_sarray = make_light_suffix_array(_entry_buf, _entry_buf_size);
  int *entry_suffix_array = entry_sarray.first;
  int entry_suffix_array_length = entry_sarray.second;
  savemem((savepath + SX_ENTRY).c_str(), _entry_buf, _entry_buf_size);
  savemem((savepath + SX_ENTRY_SARRAY).c_str(), (byte *)entry_suffix_array, sizeof(int)*entry_suffix_array_length);
  free((void *)_entry_buf); _entry_buf = NULL;
  free((void *)entry_suffix_array);

  std::pair<int*,int> jword_sarray = make_light_suffix_array(_jword_buf, _jword_buf_size);
  int *jword_suffix_array = jword_sarray.first;
  int jword_suffix_array_length = jword_sarray.second;
  savemem((savepath + SX_JWORD).c_str(), _jword_buf, _jword_buf_size);
  savemem((savepath + SX_JWORD_SARRAY).c_str(), (byte *)jword_suffix_array, sizeof(int)*jword_suffix_array_length);
  free((void *)_jword_buf); _jword_buf = NULL;
  free((void *)jword_suffix_array);

  std::pair<int*,int> example_sarray = make_light_suffix_array(_example_buf, _example_buf_size);
  int *example_suffix_array = example_sarray.first;
  int example_suffix_array_length = example_sarray.second;
  savemem((savepath + SX_EXAMPLE).c_str(), _example_buf, _example_buf_size);
  savemem((savepath + SX_EXAMPLE_SARRAY).c_str(), (byte *)example_suffix_array, sizeof(int)*example_suffix_array_length);
  free((void *)_example_buf); _example_buf = NULL;
  free((void *)example_suffix_array);

  std::pair<int*,int> pron_sarray = make_light_suffix_array(_pron_buf, _pron_buf_size);
  int *pron_suffix_array = pron_sarray.first;
  int pron_suffix_array_length = pron_sarray.second;
  savemem((savepath + SX_PRON).c_str(), _pron_buf, _pron_buf_size);
  savemem((savepath + SX_PRON_SARRAY).c_str(), (byte *)pron_suffix_array, sizeof(int)*pron_suffix_array_length);
  free((void *)_pron_buf); _pron_buf = NULL;
  free((void *)pron_suffix_array);

  std::pair<int,int> time2 = time_usec();
  printf("suffix array: {%d/%d %d/%d %d/%d %d/%d} ; real:%.3fmsec process:%.3fmsec\n",
         this->entry_suffix_array_length, _entry_buf_offset,
         this->jword_suffix_array_length, _jword_buf_offset,
         this->example_suffix_array_length, _example_buf_offset,
         this->pron_suffix_array_length, _pron_buf_offset,
         0.001*time2.first, 0.001*time2.second);

  this->load_additional_files();

  return toc_len;
}

/*
inline void save_result(const lookup_result_vec& result_vec, lookup_result result)
{
  if (result.entry_word && result.entry_word[0]) result.entry_word = clone_cstr(result.entry_word);
  if (result.jword && result.jword[0]) result.jword = clone_cstr(result.jword);
  if (result.example && result.example[0]) result.example = clone_cstr(result.example);
  if (result.pron && result.pron[0]) result.pron = clone_cstr(result.pron);

  result_vec.push_back(result);
}
*/
void cb_dump_entry(PDICDatafield *datafield)
{
  puts((char *)datafield->entry_word_utf8());
  ++match_count;
}

void cb_dump(PDICDatafield *datafield)
{
  byte *entry_word = datafield->entry_word_utf8();
  byte *jword      = datafield->jword_utf8();
  byte *example    = datafield->example_utf8();
  byte *pron       = datafield->pron_utf8();

  lookup_result result = (lookup_result){entry_word,jword,example,pron};
  render_result(&result, datafield->criteria->re2_pattern);
  save_result(_result_vec, &result);
  ++match_count;
}

void cb_save(PDICDatafield *datafield)
{
  byte *entry_word = datafield->entry_word_utf8();
  byte *jword      = datafield->jword_utf8();
  byte *example    = datafield->example_utf8();
  byte *pron       = datafield->pron_utf8();

  lookup_result result = (lookup_result){entry_word,jword,example,pron};
  save_result(_result_vec, &result);
  ++match_count;
}


lookup_result_vec
Dict::normal_lookup(byte *needle, bool exact_match)
{
  if (separator_mode) {
    std::cout << "====== ^" << needle << (exact_match ? "$" : "") << " in " << name << " ======" << std::endl;
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

  _result_vec.clear();

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
      datablock->iterate(&cb_dump, criteria);
    } else {
      datablock->iterate(&cb_save, criteria);
    }
    delete datablock;
  }
not_found:
  ;

  return lookup_result_vec(all(_result_vec));
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
        Toc *t = &toc[word_id];
        lookup_result result = (lookup_result){
          (byte*)entry_buf + t->entry_start_pos,
          (byte*)jword_buf + t->jword_start_pos,
          (byte*)example_buf + t->example_start_pos,
          (byte*)pron_buf + t->pron_start_pos};
        if (direct_dump_mode) {
          render_result(&result, &pattern);
        }
        matched_offsets.insert(word_id);
      }
    }
  }

  return matched_offsets;
}

lookup_result_vec
Dict::sarray_lookup(byte *needle)
{
  if (!toc || !entry_buf) {
    std::cout << "// [NOTICE] suffix-array検索には事前のインデックス作成が必要です。" << std::endl;
    std::cout << "//  → .make toc" << std::endl;
    return lookup_result_vec();
  }

  if (separator_mode) {
    std::cout << "====== \"" << needle << "\" in " << name << " ======" << std::endl;
  }

  _result_vec.clear();

  std::set<int> matched_word_ids;
  std::set<int> entry_matches = this->search_in_entry_sarray(needle);
  matched_word_ids.insert(all(entry_matches));
  if (full_search_mode) {
    std::set<int> jword_matches = this->search_in_jword_sarray(needle);
    matched_word_ids.insert(all(jword_matches));
    std::set<int> example_matches = this->search_in_example_sarray(needle);
    matched_word_ids.insert(all(example_matches));
    std::set<int> pron_matches = this->search_in_pron_sarray(needle);
    matched_word_ids.insert(all(pron_matches));
  }

  traverse(matched_word_ids,word_id) {
    Toc *t = &toc[*word_id];
    lookup_result result = (lookup_result){
      (byte*)entry_buf + t->entry_start_pos,
      (byte*)jword_buf + t->jword_start_pos,
      (byte*)example_buf + t->example_start_pos,
      (byte*)pron_buf + t->pron_start_pos };
    save_result(_result_vec, &result);
  }
  match_count += matched_word_ids.size();

  return lookup_result_vec(all(_result_vec));
}

lookup_result_vec
Dict::regexp_lookup(RE2 *re)
{
  if (!toc || !entry_buf) {
    std::cout << "// [NOTICE] 正規表現検索には事前のインデックス作成が必要です。" << std::endl;
    std::cout << "//  → .make toc" << std::endl;
    return lookup_result_vec();
  }

  if (separator_mode) {
    std::cout << "====== /" << re->pattern() << "/ in " << name << " ======" << std::endl;
  }

  _result_vec.clear();

  int matched_entries_count = 0;
  for (int i=0; i<toc_length; ++i) {
    const char *entry_word = (const char *)entry_buf + toc[i].entry_start_pos;
    const char *jword = (const char *)jword_buf + toc[i].jword_start_pos;
    const char *example = (const char *)example_buf + toc[i].example_start_pos;
    const char *pron = (const char *)pron_buf + toc[i].pron_start_pos;
    if (RE2::PartialMatch(entry_word, *re)
        || (full_search_mode && jword[0] && RE2::PartialMatch(jword, *re))
        || (full_search_mode && example[0] && RE2::PartialMatch(example, *re))
        || (full_search_mode && pron[0] && RE2::PartialMatch(pron, *re))
        ) {
      lookup_result result = (lookup_result){(byte*)entry_word,(byte*)jword,(byte*)example,(byte*)pron};
      if (direct_dump_mode) render_result(&result, re);
      save_result(_result_vec, &result);
      ++matched_entries_count;
    }
  }

  match_count += matched_entries_count;

  return lookup_result_vec(all(_result_vec));
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
  if (example_buf) {
    unloadmem((byte *)example_buf);
    example_buf = NULL;
  }
  if (pron_buf) {
    unloadmem((byte *)pron_buf);
    pron_buf = NULL;
  }

  if (entry_suffix_array) {
    unloadmem((byte *)entry_suffix_array);
    entry_suffix_array = (int *)NULL;
  }
  if (jword_suffix_array) {
    unloadmem((byte *)jword_suffix_array);
    jword_suffix_array = (int *)NULL;
  }
  if (example_suffix_array) {
    unloadmem((byte *)example_suffix_array);
    example_suffix_array = (int *)NULL;
  }
  if (pron_suffix_array) {
    unloadmem((byte *)pron_suffix_array);
    pron_suffix_array = (int *)NULL;
  }
}

bool
Dict::load_additional_files()
{
  unload_additional_files();

  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + this->suffix();

    if (this->toc) {
      // 読み込み済みなので無視
    } else {
      this->toc = (Toc *)loadmem((path + SX_TOC).c_str()); // < 10usec
      if (this->toc) {
        this->toc_length = mem_size((byte *)this->toc) / sizeof(Toc);

        this->entry_start_rev.clear();
        this->jword_start_rev.clear();
        this->example_start_rev.clear();
        this->pron_start_rev.clear();

        // EIJI-132 で -O3なしで 6sec前後, -O3ありで 889msec
        for (int i=0; i<this->toc_length; ++i) {
          Toc *toc = &this->toc[i];
          if (toc->entry_start_pos)
            entry_start_rev[ toc->entry_start_pos ] = i;
          if (toc->jword_start_pos)
            jword_start_rev[ toc->jword_start_pos ] = i;
          if (toc->example_start_pos)
            example_start_rev[ toc->example_start_pos ] = i;
          if (toc->pron_start_pos)
            pron_start_rev[ toc->pron_start_pos ] = i;
        }
      }
    }

    if (!this->entry_buf) {
      this->entry_buf = (byte *)loadmem((path + SX_ENTRY).c_str());
    }
    if (!this->jword_buf) {
      this->jword_buf = (byte *)loadmem((path + SX_JWORD).c_str());
    }
    if (!this->example_buf) {
      this->example_buf = (byte *)loadmem((path + SX_EXAMPLE).c_str());
    }
    if (!this->pron_buf) {
      this->pron_buf = (byte *)loadmem((path + SX_PRON).c_str());
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
    if (!this->example_suffix_array) {
      this->example_suffix_array = (int *)loadmem((path + SX_EXAMPLE_SARRAY).c_str());
      if (this->example_suffix_array) {
        this->example_suffix_array_length = mem_size((byte *)this->example_suffix_array) / sizeof(int);
      }
    }
    if (!this->pron_suffix_array) {
      this->pron_suffix_array = (int *)loadmem((path + SX_PRON_SARRAY).c_str());
      if (this->pron_suffix_array) {
        this->pron_suffix_array_length = mem_size((byte *)this->pron_suffix_array) / sizeof(int);
      }
    }
  }

  if (verbose_mode) {
    std::cout << this->suffix() << ": loaded sarray index successfully." << std::endl;
  }

  return true;
}

void render_result(lookup_result *result, RE2 *re)
{
  if (render_count >= render_count_limit) return;

  std::string entry_word((const char *)result->entry_word);
  if (ansi_coloring_mode) {
    //std::cout << "\x1b[4m" << entry_word << "\x1b[24m" << std::endl; // underline
    std::cout << ANSI_FGCOLOR_BLUE; // <blue>
    RE2::GlobalReplace(&entry_word, *re, ANSI_FGCOLOR_RED "\\0" ANSI_FGCOLOR_BLUE); // red
    std::cout << ANSI_BOLD_ON << entry_word << ANSI_BOLD_OFF; // <bold>entry_word</bold>
    if (result->pron && result->pron[0]) std::cout << " [" << result->pron << "]";
    std::cout << ANSI_FGCOLOR_DEFAULT; // </blue>
  } else {
    std::cout << entry_word;
    if (result->pron && result->pron[0]) std::cout << " [" << result->pron << "]";
  }
  std::cout << std::endl;

  std::string indent = "   ";

  if (result->jword && result->jword[0]) {
    std::string jword(indent + (const char *)result->jword);
    //RE2::GlobalReplace(&jword, "◆", "\n◆");
    RE2::GlobalReplace(&jword, "\n", "\n"+indent);
    if (ansi_coloring_mode) {
      RE2::GlobalReplace(&jword, *re, "\x1b[31m\\0\x1b[39m"); // red
    }
    std::cout << jword << std::endl;
  }
  if (result->example && result->example[0]) {
    std::string example(indent + (const char *)result->example);
    RE2::GlobalReplace(&example, "\n", "\n"+indent);
    std::cout << example << std::endl;
  }

  if (more_newline_mode) std::cout << std::endl;

  ++render_count;
}
