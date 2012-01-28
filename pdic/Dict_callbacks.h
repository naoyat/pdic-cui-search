// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDIC_DICT_CALLBACKS_H_
#define PDIC_DICT_CALLBACKS_H_

#include <re2/re2.h>

#include "util/types.h"

class PDICDatafield;

void reset_match_count();
void lap_match_count();
void say_match_count();
void reset_render_count();
void say_render_count();

// render
void render_result(lookup_result result, RE2 *re);

// callbacks
void cb_estimate_buf_size(PDICDatafield *datafield);
void cb_stock_entry_words(PDICDatafield *datafield);
void cb_dump_entry(PDICDatafield *datafield);
void cb_dump(PDICDatafield *datafield);
void cb_save(PDICDatafield *datafield);

// 英辞郎データから全変化形を取り出したい
void cb_dump_eijiro_henkakei(PDICDatafield *datafield);

#endif  // PDIC_DICT_CALLBACKS_H_
