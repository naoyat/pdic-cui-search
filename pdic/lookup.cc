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

std::vector<Dict*> dicts;
std::vector<int> current_dict_ids;

lookup_result_vec current_result_vec;
RE2* current_pattern;
std::pair<std::string, std::string> current_query;

extern bool direct_dump_mode;
int default_lookup_flags = LOOKUP_NORMAL | LOOKUP_EXACT_MATCH;

const int kMaxNeedlePatternSize = 1000;

lookup_result_vec _normal_lookup(byte *needle, int needle_len) {
  if (!needle_len) needle_len = strlen(reinterpret_cast<char*>(needle));

  bool exact_match = true;
  if (needle[needle_len-1] == '*') {
    needle[needle_len-1] = 0;
    exact_match = false;
  }
  current_query = std::make_pair(exact_match ? "exact" : "match-forward",
                                 reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        dicts[*current_dict_id]->normal_lookup(reinterpret_cast<byte*>(needle),
                                               exact_match);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _sarray_lookup(byte *needle, int needle_len) {
  current_query = std::make_pair("suffix-array",
                                 reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        dicts[*current_dict_id]->sarray_lookup(needle);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _regexp_lookup(RE2 *current_pattern) {
  current_query = std::make_pair("regexp", current_pattern->pattern());

  lookup_result_vec total_result_vec;
  traverse(current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        dicts[*current_dict_id]->regexp_lookup(current_pattern);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _full_lookup(byte *needle, int needle_len) {
  current_query = std::make_pair("full", current_pattern->pattern());

  lookup_result_vec total_result_vec;
  traverse(current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        dicts[*current_dict_id]->full_lookup(needle, current_pattern);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

void lookup(byte *needle, int needle_len, int flag) {
  if (is_empty(needle)) return;

  if (current_dict_ids.size() == 0) {
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
    current_pattern = new RE2(needle_pattern);
    result_vec = _normal_lookup(needle, needle_len);
  } else if (flag == LOOKUP_SARRAY) {
    if (needle[needle_len-1] == '*') {
      needle[needle_len-1] = 0;
    }
    current_pattern =
        new RE2(re2::StringPiece(reinterpret_cast<char*>(needle),
                                 needle_len));
    result_vec = _sarray_lookup(needle, needle_len);
  } else if (flag == LOOKUP_REGEXP) {
    current_pattern =
        new RE2(re2::StringPiece(reinterpret_cast<char*>(needle),
                                 needle_len));
    result_vec = _regexp_lookup(current_pattern);
  } else {
    current_pattern =
        new RE2(re2::StringPiece(reinterpret_cast<char*>(needle),
                                 needle_len));
    result_vec = _full_lookup(needle, needle_len);
  }

  current_result_vec.assign(all(result_vec));
  lap_match_count();
  if (!direct_dump_mode) render_current_result();
  say_match_count();
}

int current_lookup_flags() {
  return default_lookup_flags;
}

const char *current_lookup_mode() {
  if (default_lookup_flags == LOOKUP_NORMAL)
    return "normal";
  else if (default_lookup_flags == (LOOKUP_NORMAL | LOOKUP_EXACT_MATCH))
    return "exact";
  else if (default_lookup_flags == LOOKUP_SARRAY)
    return "sarray";
  else if (default_lookup_flags == LOOKUP_REGEXP)
    return "regexp";
  else
    return "all";
}
