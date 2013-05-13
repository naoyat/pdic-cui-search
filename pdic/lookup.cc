// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "pdic/lookup.h"

#include <re2/re2.h>
#include <re2/stringpiece.h>

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "Shell.h"
#include "pdic/Dict.h"
#include "pdic/Dict_callbacks.h"
#include "util/stlutil.h"
#include "util/types.h"
#include "util/util.h"

extern Shell *g_shell;
extern int dump_remain_count_;

const int kMaxNeedlePatternSize = 1000;

lookup_result_vec _pdic_match_forward_lookup(byte *needle, int flags) {
  g_shell->current_query =
      std::make_pair("match-forward", reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]->
        pdic_match_forward_lookup(needle, flags);
    total_result_vec.insert(total_result_vec.end(),
                            result_vec.begin(), result_vec.end());
  }
  std::sort(total_result_vec.begin(), total_result_vec.end(),
            lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _exact_lookup(byte *needle, int flags) {
  g_shell->current_query =
      std::make_pair("exact", reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]->exact_lookup(needle, flags);
    total_result_vec.insert(total_result_vec.end(),
                            result_vec.begin(), result_vec.end());
  }
  std::sort(total_result_vec.begin(), total_result_vec.end(),
            lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _sarray_lookup(byte *needle, int flags) {
  g_shell->current_query =
      std::make_pair("suffix-array", reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]->sarray_lookup(needle, flags);
    total_result_vec.insert(total_result_vec.end(),
                            result_vec.begin(), result_vec.end());
  }
  std::sort(total_result_vec.begin(), total_result_vec.end(),
            lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _regexp_lookup(RE2 *pattern, int flags) {
  g_shell->current_query =
      std::make_pair("regexp", pattern->pattern());

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]->regexp_lookup(pattern, flags);
    total_result_vec.insert(total_result_vec.end(),
                            result_vec.begin(), result_vec.end());
  }
  std::sort(total_result_vec.begin(), total_result_vec.end(),
            lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec _full_lookup(byte *needle, int flags) {
  g_shell->current_query =
      std::make_pair("full", reinterpret_cast<const char*>(needle));

  lookup_result_vec total_result_vec;
  traverse(g_shell->current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec =
        g_shell->dicts[*current_dict_id]
        ->full_lookup(needle, g_shell->current_pattern, flags);
    total_result_vec.insert(total_result_vec.end(),
                            result_vec.begin(), result_vec.end());
  }
  std::sort(total_result_vec.begin(), total_result_vec.end(),
            lookup_result_asc);
  return total_result_vec;
}

void lookup(byte *needle, int flags) {
  if (is_empty(needle)) return;

  if (g_shell->current_dict_ids.size() == 0) {
    printf("// 辞書が選択されていません。\n");
    return;
  }

  //
  // initialize
  //
  reset_match_count();
  reset_render_count();
  lookup_result_vec result_vec;

  dump_remain_count_ = std::numeric_limits<int>::max();

  /*
  bool match_backward = true;
  if (needle[needle_len-1] == '*') {
    needle[needle_len-1] = 0;
    match_backward = false;
    flags &= ~LOOKUP_MATCH_BACKWARD;
  } else {
    flags |= LOOKUP_MATCH_BACKWARD;
  }
  */

  if (!(flags & LOOKUP_CASE_SENSITIVE)) {
    std::string ci("(?i)");
    g_shell->current_pattern = new RE2(ci + reinterpret_cast<char*>(needle));
  } else {
    g_shell->current_pattern = new RE2(reinterpret_cast<char*>(needle));
  }

  if (flags & LOOKUP_PDIC_INDEX) {
    result_vec = _pdic_match_forward_lookup(needle, flags);
  } else if (flags & LOOKUP_SARRAY) {
    result_vec = _sarray_lookup(needle, flags);
  } else if (flags & LOOKUP_REGEXP) {
    result_vec = _regexp_lookup(g_shell->current_pattern, flags);
  } else if (flags & LOOKUP_FROM_ALL) {
    result_vec = _full_lookup(needle, flags);
  } else {
    if (flags & LOOKUP_EXACT_MATCH)
      result_vec = _exact_lookup(needle, flags);
    else
      return;
  }

  g_shell->current_result_vec.assign(result_vec.begin(), result_vec.end());
  lap_match_count();
  if (!g_shell->params.direct_dump_mode) g_shell->render_current_result();
  say_match_count();
}
