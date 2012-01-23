// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "pdic/Dict.h"

#include <libgen.h>
#include <strings.h>
#include <re2/re2.h>

#include <algorithm>
#include <string>
#include <vector>

#include "pdic/Criteria.h"
#include "pdic/PDICDatablock.h"
#include "pdic/PDICDatafield.h"
#include "pdic/PDICHeader.h"
#include "pdic/PDICIndex.h"
#include "util/ansi_color.h"
#include "util/bocu1.h"
#include "util/charcode.h"
#include "util/filemem.h"
#include "util/macdic_xml.h"
#include "util/search.h"
#include "util/stlutil.h"
#include "util/timeutil.h"
#include "util/utf8.h"
#include "util/util.h"
#include "util/Shell.h"

const char *sx_buf[F_COUNT] = {
  ".entry", ".trans", ".exmp", ".pron"
};
const char *sx_sarray[F_COUNT] = {
  ".entry.sf", ".trans.sf", ".exmp.sf", ".pron.sf"
};

bool lookup_result_asc(const lookup_result& left, const lookup_result& right) {
  // printf("lr/a, %s, %s\n",
  //        (const char*)left[F_ENTRY], (const char*)right[F_ENTRY]);
  return bstrcmp(left[F_ENTRY], right[F_ENTRY]) < 0;
}

bool lookup_result_desc(const lookup_result& left, const lookup_result& right) {
  // printf("lr/d, %s, %s\n",
  //        (const char*)left[F_ENTRY], (const char*)right[F_ENTRY]);
  return bstrcmp(left[F_ENTRY], right[F_ENTRY]) > 0;
}

bool toc_asc(const Toc& left, const Toc& right) {
  return left.pdic_datafield_pos < right.pdic_datafield_pos;
}

extern Shell *g_shell;

bool render_count_limit_exceeded = false;
bool said_that = false;
int match_count, render_count;
int _search_lap_usec, _render_lap_usec;

std::set<int> _result_id_set;
Dict *_dict;

void reset_match_count() {
  time_reset();
  match_count = 0;
}

void reset_render_count() {
  time_reset();
  render_count = 0;
  render_count_limit_exceeded = false;
  said_that = false;
}

void lap_match_count() {
  std::pair<int, int> search_lap = time_usec();
  _search_lap_usec = search_lap.first;

  time_reset();
}

void say_match_count() {
  std::pair<int, int> render_lap = time_usec();
  _render_lap_usec = render_lap.first;

  printf("%s", ANSI_FGCOLOR_GREEN);
  if (render_count >= g_shell->params.render_count_limit
      && g_shell->params.stop_on_limit_mode) {
    printf("// 検索結果の最初の%d件を表示", render_count);
  } else {
    printf("// 結果%d件", match_count);
    if (render_count < match_count) printf(" (うち%d件表示)", render_count);
  }
  if (g_shell->params.direct_dump_mode) {
    printf(", 検索+表示:%.3fミリ秒", 0.001 * _search_lap_usec);
  } else {
    printf(", 検索:%.3fミリ秒", 0.001 * _search_lap_usec);
    printf(", 表示:%.3fミリ秒", 0.001 * _render_lap_usec);
  }
  printf(".\n" ANSI_FGCOLOR_DEFAULT);
}

void say_render_count() {
  std::pair<int, int> render_lap = time_usec();
  _render_lap_usec = render_lap.first;

  printf(ANSI_FGCOLOR_GREEN "// 結果%d件", match_count);
  // if (_match_count >= 2) putchar('s');
  if (render_count < match_count) printf(" (うち%d件表示)", render_count);
  printf(", 表示:%.3fミリ秒", 0.001 * _render_lap_usec);
  printf(".\n" ANSI_FGCOLOR_DEFAULT);
}

// ctor
Dict::Dict(const std::string& name, byte *filemem) {
  index = new PDICIndex(filemem);
  this->name = name;
  this->filemem = filemem;

  toc = static_cast<Toc*>(NULL);
  for (int field = 0; field < F_COUNT; ++field) {
    dict_buf[field] = static_cast<byte*>(NULL);
    dict_suffix_array[field] = static_cast<int*>(NULL);
  }

  _prefix = new char[name.size()+1];
  snprintf(_prefix, name.size()+1, "%s",
           basename(const_cast<char*>(name.c_str())));

  int len = strlen(_prefix);
  if (strcasecmp(_prefix + len-4, ".dic") == 0) {
    _prefix[len - 4] = 0;
  }

  load_additional_files();
}

Dict::~Dict() {
  unloadmem(filemem);
  filemem = static_cast<byte*>(NULL);

  delete _prefix;
  delete index;

  unload_additional_files();
}

// global (only for callback routines)
// 全ての見出し語（に'\0'を付加したデータ）を結合したもの
// 訳語、用例、発音記号についても同様
byte *_dict_buf[F_COUNT];
int _dict_buf_size[F_COUNT], _dict_buf_offset[F_COUNT];
int _count = 0;

// 各見出し語の開始位置など (dict_buf[f] + dict_start_pos[f][i])
std::vector<Toc> _toc;

void cb_estimate_buf_size(PDICDatafield *datafield) {
  for (int f = 0; f < F_COUNT; ++f) {
    byte *in_utf8 = datafield->in_utf8(f);
    if (is_not_empty(in_utf8)) {
      _dict_buf_size[f] += strlen(reinterpret_cast<char*>(in_utf8)) + 1;
    }
  }

  ++_count;

  if (_count & 1000 == 0) {
    for (int f = 0; f < F_COUNT; ++f) {
      printf("%9d ", _dict_buf_size[f]);
    }
    putchar('\r');
  }
}

void cb_stock_entry_words(PDICDatafield *datafield) {
  Toc toc;

  toc.pdic_datafield_pos = datafield->start_pos;

  for (int f = 0; f < F_COUNT; ++f) {
    byte *in_utf8 = datafield->in_utf8(f);
    if (is_not_empty(in_utf8)) {
      int memsize_in_utf8 = strlen(reinterpret_cast<char*>(in_utf8)) + 1;
      memcpy(_dict_buf[f] + _dict_buf_offset[f], in_utf8, memsize_in_utf8);
      // returns the position at \0

      toc.start_pos[f] = _dict_buf_offset[f];
      _dict_buf_offset[f] += memsize_in_utf8;
    } else {
      toc.start_pos[f] = 0;
    }
  }

  _toc.push_back(toc);
}

int Dict::make_macdic_xml() {
  macdic_xml_open(std::string(this->prefix()) + SX_XML);
  index->iterate_all_datablocks(&cb_macdic_xml, NULL);
  macdic_xml_close();

  return 0;
}

int Dict::make_toc() {
  printf("%s: インデックスを作成します...\n", name.c_str());

  // 現在使用中のインデックスをアンロードしてから
  this->unload_additional_files();

  time_reset();

  // initialize

  //
  // PASS1: 先に必要なメモリ量を計算してしまう
  //
  _count = 0;
  for (int field = 0; field < F_COUNT; ++field) {
    _dict_buf[field] = NULL;
    _dict_buf_size[field] = 1;
  }

  index->iterate_all_datablocks(&cb_estimate_buf_size, NULL);

  for (int field = 0; field < F_COUNT; ++field) {
    _dict_buf[field] = static_cast<byte*>(malloc(_dict_buf_size[field]));
    if (!_dict_buf[field]) {
      printf("_dict_buf[%d] is not allocated\n", field);
      for (int past = 0; past < field; ++past) {
        free(static_cast<void*>(_dict_buf[past]));
      }
      return -1;
    }
  }

  //
  // PASS2: 全てのデータブロックから全ての単語データを抽出
  //
  _toc.clear();

  for (int field = 0; field < F_COUNT; ++field) {
    _dict_buf[field][0] = 0;
    _dict_buf_offset[field] = 1;
  }

  index->iterate_all_datablocks(&cb_stock_entry_words, NULL);

  std::pair<int, int> time = time_usec();
  printf("データ抽出: %d words, {%d + %d + %d + %d};",
         static_cast<int>(_toc.size()),
         _dict_buf_size[F_ENTRY],
         _dict_buf_size[F_JWORD],
         _dict_buf_size[F_EXAMPLE],
         _dict_buf_size[F_PRON]);
  printf("real:%.3fmsec process:%.3fmsec\n",
         0.001*time.first, 0.001*time.second);

  time_reset();

  //
  // 保存
  //
  std::string savepath = this->prefix();

  // TOC
  int toc_len = _toc.size();
  Toc *toc = static_cast<Toc*>(malloc(sizeof(Toc)*toc_len));
  if (!toc) {
    printf("TOC is not allocated\n");
    return -1;
  }

  std::sort(all(_toc), toc_asc);

  for (int i = 0, c = _toc.size(); i < c; ++i)
    toc[i] = _toc[i];

  savemem((savepath + SX_TOC).c_str(),
          reinterpret_cast<byte*>(toc),
          sizeof(Toc)*_toc.size());
  free(static_cast<void*>(toc));
  _toc.clear();

  // entry sarray
  for (int f = 0; f < F_COUNT; ++f) {
    std::pair<int*, int> sarray =
        make_light_suffix_array(_dict_buf[f], _dict_buf_size[f]);
    int *suffix_array = sarray.first, suffix_array_length = sarray.second;
    savemem((savepath + sx_buf[f]).c_str(), _dict_buf[f], _dict_buf_size[f]);
    savemem((savepath + sx_sarray[f]).c_str(),
            reinterpret_cast<byte*>(suffix_array),
            sizeof(*suffix_array)*suffix_array_length);

    free(static_cast<void*>(_dict_buf[f]));
    _dict_buf[f] = NULL;
    free(static_cast<void*>(suffix_array));
  }

  std::pair<int, int> time2 = time_usec();
  printf("suffix array: {");
  for (int f = 0; f < F_COUNT; ++f) {
    printf(" %d/%d", this->dict_suffix_array_length[f], _dict_buf_offset[f]);
  }
  printf(" } ; real:%.3fmsec process:%.3fmsec\n",
         0.001*time2.first, 0.001*time2.second);

  this->load_additional_files();

  return toc_len;
}

void cb_dump_entry(PDICDatafield *datafield) {
  puts(reinterpret_cast<char*>(datafield->in_utf8(F_ENTRY)));

  if (++match_count >= g_shell->params.render_count_limit)
    render_count_limit_exceeded = true;
}

void cb_dump(PDICDatafield *datafield) {
  byte *fields[F_COUNT] = {
    datafield->in_utf8(F_ENTRY),
    datafield->in_utf8(F_JWORD),
    datafield->in_utf8(F_EXAMPLE),
    datafield->in_utf8(F_PRON)
  };
  render_result((lookup_result)fields, datafield->criteria->re2_pattern);

  int word_id = _dict->word_id_for_pdic_datafield_pos(datafield->start_pos);
  _result_id_set.insert(word_id);

  if (++match_count >= g_shell->params.render_count_limit)
    render_count_limit_exceeded = true;
}

void cb_save(PDICDatafield *datafield) {
  int word_id = _dict->word_id_for_pdic_datafield_pos(datafield->start_pos);
  _result_id_set.insert(word_id);

  if (++match_count >= g_shell->params.render_count_limit)
    render_count_limit_exceeded = true;
}

int Dict::word_id_for_pdic_datafield_pos(int pdic_datafield_pos) {
  if (found(revmap_pdic_datafield_pos, pdic_datafield_pos)) {
    return revmap_pdic_datafield_pos[pdic_datafield_pos];
  }
  if (pdic_datafield_pos < toc[0].pdic_datafield_pos
      || toc[this->toc_length-1].pdic_datafield_pos < pdic_datafield_pos) {
    return -1;
  }

  int lo = 0, hi = this->toc_length-1, mid = -1;

  while (lo <= hi) {
    mid = (lo + hi) / 2;  // 4億語とか扱わないから31bit桁あふれケアはしない
    int cmp = pdic_datafield_pos - toc[mid].pdic_datafield_pos;
    if (cmp == 0) {
      return revmap_pdic_datafield_pos[pdic_datafield_pos] = mid;
    }
    if (cmp < 0) {
      hi = mid - 1;
    }
    if (cmp > 0) {
      lo = mid + 1;
    }
  }

  return -1;
}

std::set<int> Dict::normal_lookup_ids(byte* needle, bool exact_match) {
  if (g_shell->params.separator_mode) {
    printf("====== ^%s%s in %s ======\n",
           reinterpret_cast<char*>(needle),
           exact_match ? "$" : "",
           name.c_str());
  }
  if (g_shell->params.stop_on_limit_mode) {
    if (render_count_limit_exceeded) {
      if (g_shell->params.verbose_mode && !said_that) {
        printf("[stop on limit] "
               "件数(%d)が制限(%d)に達しているので%sからの検索を行いません。\n",
               render_count, g_shell->params.render_count_limit, name.c_str());
        said_that = true;
      }
      return std::set<int>();
    }
  }

  int target_charcode = index->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS;

  // Criteria *criteria;
  // if (index->header->major_version() >= HYPER6) {
  Criteria *criteria = new Criteria(needle, target_charcode, exact_match);
  byte *needle_for_index = criteria->needle_for_index ?
      criteria->needle_for_index : criteria->needle;
  // printf("needle for index: {%s}\n", needle_for_index);
  bsearch_result_t result = index->bsearch_in_index(needle_for_index,
                                                    exact_match);

  int from, to;
  if (result.first) {
    from = result.second.first;  // - 1; if (from < 0) from = 0;
    if (bstrcmp(index->entry_word(from), criteria->needle) != 0) {
      --from;
      if (from < 0) from = 0;
    }
    to = result.second.second;
  } else {
    from = result.second.second - 1;
    if (from < 0) goto not_found;
    to = from;
  }

  if (g_shell->params.verbose_mode) {
    // printf("lookup. from %d to %d, %d/%d...\n",
    //        from, to, to-from+1, index->_nindex);
  }

  _result_id_set.clear();
  _dict = this;

  for (int ix = from; ix <= to; ix++) {
    if (g_shell->params.stop_on_limit_mode) {
      if (render_count_limit_exceeded) {
        if (g_shell->params.verbose_mode && !said_that) {
          printf("[stop on limit] "
                 "件数(%d)が制限(%d)に達したので検索を中断します。\n",
                 render_count, g_shell->params.render_count_limit);
          said_that = true;
        }
        break;
      }
    }

    if (g_shell->params.verbose_mode) {
      /*
      byte *utf8str = bocu1_to_utf8( index->entry_word(ix) );
      printf("  [%d/%d] %s\n", ix, index->_nindex, utf8str);
      free((void *)utf8str);
      */
    }
    if (ix < 0) continue;
    if (ix >= index->_nindex) break;

    PDICDatablock* datablock = new PDICDatablock(filemem, this->index, ix);
    if (g_shell->params.direct_dump_mode) {
      datablock->iterate(&cb_dump, criteria);
    } else {
      datablock->iterate(&cb_save, criteria);
    }
    delete datablock;
  }
 not_found:
  {
  }

  _dict = NULL;

  return std::set<int>(all(_result_id_set));
}


std::set<int> Dict::search_in_sarray(int field, byte *needle) {
  byte *buf = dict_buf[field];
  int *sarray = dict_suffix_array[field];
  int sarray_length = dict_suffix_array_length[field];

  bsearch_result_t result = search(buf, sarray, sarray_length, needle, false);

  std::set<int> matched_offsets;

  if (result.first) {
    RE2 pattern((const char *)needle);
    for (int i = result.second.first; i <= result.second.second; ++i) {
      if (g_shell->params.stop_on_limit_mode) {
        if (render_count_limit_exceeded) {
          if (g_shell->params.verbose_mode && !said_that) {
            printf("[stop on limit] "
                   "件数(%d)が制限(%d)に達したので検索を中断します。\n",
                   render_count, g_shell->params.render_count_limit);
            said_that = true;
          }
          break;
        }
      }
      byte *word = strhead(buf + sarray[i]);
      int offset = static_cast<int>(word - buf);
      int word_id = rev(field, offset);
      if (word_id >= 0) {
        Toc *t = &toc[word_id];
        byte *fields[F_COUNT] = {
          dict_buf[F_ENTRY]   + t->start_pos[F_ENTRY],
          dict_buf[F_JWORD]   + t->start_pos[F_JWORD],
          dict_buf[F_EXAMPLE] + t->start_pos[F_EXAMPLE],
          dict_buf[F_PRON]    + t->start_pos[F_PRON]
        };
        if (g_shell->params.direct_dump_mode) {
          render_result((lookup_result)fields, &pattern);
        }
        matched_offsets.insert(word_id);
      }
    }
  }

  return matched_offsets;
}

lookup_result_vec Dict::normal_lookup(byte *needle, bool exact_match) {
  std::set<int> matched_word_ids = normal_lookup_ids(needle, exact_match);
  return ids_to_result(matched_word_ids);
}

lookup_result_vec Dict::sarray_lookup(byte *needle) {
  std::set<int> matched_word_ids = sarray_lookup_ids(needle);
  return ids_to_result(matched_word_ids);
}

lookup_result_vec Dict::regexp_lookup(RE2 *re) {
  std::set<int> matched_word_ids = regexp_lookup_ids(re);
  return ids_to_result(matched_word_ids);
}

lookup_result_vec Dict::full_lookup(byte *needle, RE2 *re) {
  std::set<int> matched_word_ids;

  std::set<int> matched_word_ids_normal = normal_lookup_ids(needle, false);
  matched_word_ids.insert(matched_word_ids_normal.begin(),
                          matched_word_ids_normal.end());

  std::set<int> matched_word_ids_sarray = sarray_lookup_ids(needle);
  matched_word_ids.insert(matched_word_ids_sarray.begin(),
                          matched_word_ids_sarray.end());

  std::set<int> matched_word_ids_regexp = regexp_lookup_ids(re);
  matched_word_ids.insert(matched_word_ids_regexp.begin(),
                          matched_word_ids_regexp.end());

  return ids_to_result(matched_word_ids);
}

lookup_result_vec Dict::ids_to_result(const std::set<int>& word_ids) {
  lookup_result_vec result_vec;

  traverse(word_ids, word_id) {
    if (*word_id < 0) continue;

    Toc *t = &toc[*word_id];
    // printf("%d: [ %d %d %d %d %d ]\n",
    //        *word_id,
    //        t->pdic_datafield_pos,
    //        t->start_pos[0],
    //        t->start_pos[1],
    //        t->start_pos[2],
    //        t->start_pos[3]);

    byte *fields[F_COUNT] = {
      dict_buf[F_ENTRY]   + t->start_pos[F_ENTRY],
      dict_buf[F_JWORD]   + t->start_pos[F_JWORD],
      dict_buf[F_EXAMPLE] + t->start_pos[F_EXAMPLE],
      dict_buf[F_PRON]    + t->start_pos[F_PRON]
    };

    result_vec.push_back((lookup_result)clone(fields, sizeof(fields[0])*4));
  }

  return result_vec;
}

std::set<int> Dict::sarray_lookup_ids(byte *needle) {
  if (!toc || !dict_buf[F_ENTRY]) {
    printf("// [NOTICE] "
           "suffix-array検索には事前のインデックス作成が必要です。\n");
    printf("//  → .make toc\n");
    return std::set<int>();
  }

  if (g_shell->params.separator_mode) {
    printf("====== \"%s\" in %s ======\n",
           reinterpret_cast<char*>(needle),
           name.c_str());
  }

  std::set<int> matched_word_ids;
  for (int f = 0; f < F_COUNT; ++f) {
    if (g_shell->params.stop_on_limit_mode) {
      if (render_count_limit_exceeded) {
        if (g_shell->params.verbose_mode && !said_that) {
          printf("[stop on limit] "
                 "件数(%d)が制限(%d)に達したので検索を中断します。\n",
                 render_count, g_shell->params.render_count_limit);
          said_that = true;
        }
        break;
      }
    }
    if (f == F_ENTRY || g_shell->params.full_search_mode) {
      std::set<int> matched_id_set = this->search_in_sarray(f, needle);
      matched_word_ids.insert(all(matched_id_set));
    }
  }
  return matched_word_ids;
}

std::set<int> Dict::regexp_lookup_ids(RE2 *re) {
  if (!toc || !dict_buf[F_ENTRY]) {
    printf("// [NOTICE] "
           "正規表現検索には事前のインデックス作成が必要です。\n");
    printf("//  → .make toc\n");
    return std::set<int>();
  }

  if (g_shell->params.separator_mode) {
    printf("====== /%s/ in %s ======\n",
           re->pattern().c_str(),
           name.c_str());
  }

  std::set<int> matched_word_ids;

  for (int word_id = 0; word_id < toc_length; ++word_id) {
    if (g_shell->params.stop_on_limit_mode) {
      if (render_count_limit_exceeded) {
        if (g_shell->params.verbose_mode && !said_that) {
          printf("[stop on limit] "
                 "件数(%d)が制限(%d)に達したので検索を中断します。\n",
                 render_count, g_shell->params.render_count_limit);
          said_that = true;
        }
        break;
      }
    }
    byte *fields[F_COUNT] = {
      dict_buf[F_ENTRY]   + toc[word_id].start_pos[F_ENTRY],
      dict_buf[F_JWORD]   + toc[word_id].start_pos[F_JWORD],
      dict_buf[F_EXAMPLE] + toc[word_id].start_pos[F_EXAMPLE],
      dict_buf[F_PRON]    + toc[word_id].start_pos[F_PRON]
    };
    if (RE2::PartialMatch((const char *)fields[F_ENTRY], *re)
        || (g_shell->params.full_search_mode
            && ((fields[F_JWORD][0]
                 && RE2::PartialMatch((const char *)fields[F_JWORD],
                                      *re))
                || (fields[F_EXAMPLE][0]
                    && RE2::PartialMatch((const char *)fields[F_EXAMPLE],
                                         *re))
                || (fields[F_PRON][0]
                    && RE2::PartialMatch((const char *)fields[F_PRON],
                                         *re))))) {
      if (g_shell->params.direct_dump_mode) render_result(fields, re);
      matched_word_ids.insert(word_id);

      if (++match_count >= g_shell->params.render_count_limit)
        render_count_limit_exceeded = true;
    }
  }

  return matched_word_ids;
}

void Dict::unload_additional_files() {
  if (toc) {
    unloadmem(reinterpret_cast<byte*>(toc));
    toc = static_cast<Toc*>(NULL);
  }

  for (int field = 0; field < F_COUNT; ++field) {
    if (dict_buf[field]) {
      unloadmem(reinterpret_cast<byte*>(dict_buf[field]));
      dict_buf[field] = NULL;
    }
    if (dict_suffix_array[field]) {
      unloadmem(reinterpret_cast<byte*>(dict_suffix_array[field]));
      dict_suffix_array[field] = static_cast<int*>(NULL);
    }
  }
}

int Dict::rev(int field, int pos) {
  std::pair<int, int> key = std::make_pair(field, pos);
  if (found(revmap, key)) return revmap[key];
  if (pos < toc[0].start_pos[field]
      || toc[toc_length-1].start_pos[field] < pos) {
    return -1;
  }

  int lo = 0, hi = this->toc_length - 1, mid = -1;
  while (lo <= hi) {
    mid = (lo + hi) / 2;  // 4億語とか扱わないから31bit桁あふれケアはしない
    int cmp = pos - toc[mid].start_pos[field];
    if (cmp == 0) {
      return revmap[key] = mid;
    }
    if (cmp < 0) {
      hi = mid - 1;
    }
    if (cmp > 0) {
      lo = mid + 1;
    }
  }

  return -1;
}

bool Dict::load_additional_files() {
  unload_additional_files();

  for (uint i = 0; i < g_shell->loadpaths.size(); ++i) {
    std::string path = g_shell->loadpaths[i] + "/" + this->prefix();

    if (this->toc) {
      // 読み込み済みなので無視
    } else {
      // < 10usec
      this->toc = reinterpret_cast<Toc*>(loadmem((path + SX_TOC).c_str()));

      if (this->toc) {
        this->toc_length = mem_size(reinterpret_cast<byte*>(this->toc))
            / sizeof(Toc);
        this->revmap.clear();
      }
    }

    for (int field = 0; field < F_COUNT; ++field) {
      if (!this->dict_buf[field]) {
        this->dict_buf[field] =
            static_cast<byte*>(loadmem((path + sx_buf[field]).c_str()));
      }
      if (!this->dict_suffix_array[field]) {
        int *suffix_array_buf =
            reinterpret_cast<int*>(loadmem((path + sx_sarray[field]).c_str()));
        if (suffix_array_buf) {
          this->dict_suffix_array_length[field] =
              mem_size(reinterpret_cast<byte*>(suffix_array_buf))
              / sizeof(*suffix_array_buf);
          this->dict_suffix_array[field] = suffix_array_buf;
        }
      }
    }
  }

  if (g_shell->params.verbose_mode) {
    printf("%s: loaded sarray index successfully.\n", this->prefix());
  }

  return true;
}

void render_result(lookup_result fields, RE2 *re) {
  if (render_count >= g_shell->params.render_count_limit) {
    if (g_shell->params.verbose_mode && !said_that) {
      printf("表示件数(%d)が制限(%d)に達したので表示を中断します。\n",
             render_count, g_shell->params.render_count_limit);
      said_that = true;
    }
    render_count_limit_exceeded = true;
    return;
  }

  std::string entry_str((const char *)fields[F_ENTRY]);

  if (g_shell->params.ansi_coloring_mode) {
    printf("%s", ANSI_FGCOLOR_BLUE);
    RE2::GlobalReplace(&entry_str, *re,
                       ANSI_FGCOLOR_RED "\\0" ANSI_FGCOLOR_BLUE);
    printf("%s%s%s", ANSI_BOLD_ON, entry_str.c_str(), ANSI_BOLD_OFF);
    if (is_not_empty(fields[F_PRON]))
      printf(" [%s]", reinterpret_cast<char*>(fields[F_PRON]));
    printf("%s", ANSI_FGCOLOR_DEFAULT);
  } else {
    printf("%s", entry_str.c_str());
    if (is_not_empty(fields[F_PRON]))
      printf(" [%s]", reinterpret_cast<char*>(fields[F_PRON]));
  }
  printf("\n");

  std::string indent = "   ";

  if (is_not_empty(fields[F_JWORD])) {
    std::string jword_str(indent + (const char *)fields[F_JWORD]);
    // RE2::GlobalReplace(&jword, "◆", "\n◆");
    RE2::GlobalReplace(&jword_str, "\n", "\n"+indent);
    if (g_shell->params.ansi_coloring_mode) {
      RE2::GlobalReplace(&jword_str, *re,
                         ANSI_FGCOLOR_RED "\\0" ANSI_FGCOLOR_DEFAULT);
    }
    printf("%s\n", jword_str.c_str());
  }
  if (is_not_empty(fields[F_EXAMPLE])) {
    std::string example_str(indent + (const char *)fields[F_EXAMPLE]);
    RE2::GlobalReplace(&example_str, "\n", "\n"+indent);
    printf("%s\n", example_str.c_str());
  }

  if (g_shell->params.more_newline_mode) printf("\n");

  ++render_count;
}