// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "pdic/lookup.h"

#include <re2/re2.h>
#include <re2/stringpiece.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "pdic/Dict.h"
#include "util/stlutil.h"
#include "util/types.h"
#include "util/util.h"
#include "util/Shell.h"

extern Shell *g_shell;

const int kMaxNeedlePatternSize = 1000;

lookup_result_vec _normal_lookup(byte *needle, int needle_len) {
  if (!needle_len) needle_len = strlen(reinterpret_cast<char*>(needle));

  bool exact_match = true;
  if (needle[needle_len-1] == '*') {
    needle[needle_len-1] = 0;
    exact_match = false;
  }
  g_shell->current_query =
      std::make_pair(exact_match ? "exact" : "match-forward",
                     reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]->
        normal_lookup(reinterpret_cast<byte*>(needle),
                      exact_match);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _sarray_lookup(byte *needle, int needle_len) {
  g_shell->current_query =
      std::make_pair("suffix-array", reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]->sarray_lookup(needle);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _regexp_lookup(RE2 *pattern) {
  g_shell->current_query =
      std::make_pair("regexp", pattern->pattern());

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]->regexp_lookup(pattern);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _full_lookup(byte *needle, int needle_len) {
  g_shell->current_query =
      std::make_pair("full", reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]
        ->full_lookup(needle, g_shell->current_pattern);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

void lookup(byte *needle, int needle_len, int flag) {
  if (is_empty(needle)) return;

  if (g_shell->current_dict_ids.size() == 0) {
    printf("// 辞書が選択されていません。\n");
    return;
  }

  if (!needle_len) needle_len = strlen(reinterpret_cast<char*>(needle));

  //
  // initialize
  //
  reset_match_count();
  reset_render_count();
  lookup_result_vec result_vec;

  if (flag == LOOKUP_NORMAL || flag == (LOOKUP_NORMAL | LOOKUP_EXACT_MATCH)) {
    char needle_pattern[kMaxNeedlePatternSize];
    snprintf(needle_pattern, kMaxNeedlePatternSize, "(?i)%s", needle);
    if (needle[needle_len-1] == '*') {
      needle_pattern[4+needle_len-1] = 0;
    }
    g_shell->current_pattern = new RE2(needle_pattern);
    result_vec = _normal_lookup(needle, needle_len);
  } else if (flag == LOOKUP_SARRAY) {
    if (needle[needle_len-1] == '*') {
      needle[needle_len-1] = 0;
    }
    g_shell->current_pattern =
        new RE2(re2::StringPiece(reinterpret_cast<char*>(needle),
                                 needle_len));
    result_vec = _sarray_lookup(needle, needle_len);
  } else if (flag == LOOKUP_REGEXP) {
    g_shell->current_pattern =
        new RE2(re2::StringPiece(reinterpret_cast<char*>(needle),
                                 needle_len));
    result_vec = _regexp_lookup(g_shell->current_pattern);
  } else {
    g_shell->current_pattern =
        new RE2(re2::StringPiece(reinterpret_cast<char*>(needle),
                                 needle_len));
    result_vec = _full_lookup(needle, needle_len);
  }

  g_shell->current_result_vec.assign(all(result_vec));
  lap_match_count();
  if (!g_shell->params.direct_dump_mode) g_shell->render_current_result();
  say_match_count();
}

void default_lookup(byte *needle, int needle_len) {
  lookup(needle, needle_len, g_shell->params.default_lookup_flags);
}
