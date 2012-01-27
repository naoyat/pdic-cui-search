// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/alt.h"

#include <re2/re2.h>
#include "sandbox/Word.h"

bool alt_simple = true;

//
// alternative renderers
//
void render_result_alt(lookup_result fields, RE2 *re) {
  Word word(fields);
  if (alt_simple)
    word.render();
  else
    word.render_full();
}
