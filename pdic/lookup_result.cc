// Copyright 2012-2013 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "pdic/lookup_result.h"
#include "pdic/PDICDatafield.h"

/*
lookup_result::lookup_result(byte *entry,
                             byte *jword,
                             byte *example,
                             byte *pron,
                             int level) {
  this->entry   = entry;
  this->jword   = jword;
  this->example = example;
  this->pron    = pron;
  this->level   = level;
}
*/

lookup_result::lookup_result(PDICDatafield *datafield) {
  entry   = datafield->in_utf8(F_ENTRY);
  jword   = datafield->in_utf8(F_JWORD);
  example = datafield->in_utf8(F_EXAMPLE);
  pron    = datafield->in_utf8(F_PRON);
  level   = datafield->entry_word_level;
}

lookup_result::lookup_result(byte* dict_buf[],
                             int start_pos[],
                             int level) {
  this->entry   = dict_buf[F_ENTRY]   + start_pos[F_ENTRY];
  this->jword   = dict_buf[F_JWORD]   + start_pos[F_JWORD];
  this->example = dict_buf[F_EXAMPLE] + start_pos[F_EXAMPLE];
  this->pron    = dict_buf[F_PRON]    + start_pos[F_PRON];
  this->level   = level;
}

lookup_result::~lookup_result() {
}
