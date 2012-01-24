// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "pdic/Dict_callbacks.h"

#include <string.h>
#include <re2/re2.h>

#include <set>
#include <utility>
#include <vector>

#include "pdic/Criteria.h"
#include "pdic/Dict.h"
#include "pdic/PDICDatafield.h"
#include "util/ansi_color.h"
#include "util/types.h"
#include "util/Shell.h"
#include "util/timeutil.h"
#include "util/util.h"

extern Shell *g_shell;

// global (only for callback routines)
// 全ての見出し語（に'\0'を付加したデータ）を結合したもの
// 訳語、用例、発音記号についても同様
int _count = 0;
byte *_dict_buf[F_COUNT];
int _dict_buf_size[F_COUNT], _dict_buf_offset[F_COUNT];

// 各見出し語の開始位置など (dict_buf[f] + dict_start_pos[f][i])
std::vector<Toc> _toc;
std::set<int> _result_id_set;

Dict *_dict;

//
// match & render counts
//
int match_count, render_count;
bool render_count_limit_exceeded = false;
bool said_that = false;
int _search_lap_usec, _render_lap_usec;

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
  if (render_count < match_count) printf(" (うち%d件表示)", render_count);
  printf(", 表示:%.3fミリ秒", 0.001 * _render_lap_usec);
  printf(".\n" ANSI_FGCOLOR_DEFAULT);
}

//
// render result
//
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

//
// callbacks
//
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

