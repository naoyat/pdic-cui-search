// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./Criteria.h"

#include <stdlib.h>
#include <string.h>
#include <re2/re2.h>

#include <sstream>
#include <string>

#include "pdic/PDICDatafield.h"
#include "pdic/PDICHeader.h"
#include "pdic/PDICIndex.h"  // string_for_index()
#include "util/bocu1.h"
#include "util/charcode.h"
#include "util/dump.h"
#include "util/types.h"
#include "util/utf8.h"
#include "util/util.h"

Criteria::Criteria(byte *needle_utf8, int target_charcode, bool exact_match) {
  this->re2_pattern = new RE2(std::string("(?i)")
                              + reinterpret_cast<char*>(needle_utf8));

  this->needle_utf8 = needle_utf8;
  this->needle_for_index_utf8 = static_cast<byte*>(NULL);

  switch (target_charcode) {
    case CHARCODE_BOCU1: {
      needle = utf8_to_bocu1(needle_utf8);
      needle_size = strlen(reinterpret_cast<char*>(needle));

      this->needle_for_index_utf8 = string_for_index(needle_utf8);
      needle_for_index = utf8_to_bocu1(this->needle_for_index_utf8);
      needle_size_for_index = strlen(reinterpret_cast<char*>(needle_for_index));
    }
      break;
    case CHARCODE_SHIFTJIS:
      needle = utf8_to_sjis(needle_utf8);
      needle_size = strlen(reinterpret_cast<char*>(needle));
      needle_for_index = static_cast<byte*>(NULL);
      needle_size_for_index = 0;
      break;
    default:
      needle_size = strlen(reinterpret_cast<char*>(needle_utf8));
      needle_for_index = static_cast<byte*>(NULL);
      needle_size_for_index = 0;
      needle = clone_cstr(needle_utf8, needle_size, false);
      break;
  }
  this->exact_match = exact_match;
}

Criteria::~Criteria() {
  free(static_cast<void*>(needle));
  if (needle_for_index) free(static_cast<void*>(needle_for_index));
  if (needle_for_index_utf8) free(static_cast<void*>(needle_for_index_utf8));
  delete re2_pattern;
}

bool Criteria::match(PDICDatafield *field) {
  if (exact_match) {
    if (field->entry_word_size == needle_size
        && strncmp(reinterpret_cast<char*>(field->entry_word),
                   reinterpret_cast<char*>(needle), needle_size) == 0) {
      return true;
    } else if (field->v6index &&
             (field->entry_index_size == needle_size_for_index
              && strncmp(reinterpret_cast<char*>(field->entry_index),
                         reinterpret_cast<char*>(needle_for_index),
                         needle_size_for_index) == 0)) {
      return true;
    } else {
      return false;
    }
  } else {
    if (strncmp(reinterpret_cast<char*>(field->entry_word),
                reinterpret_cast<char*>(needle),
                needle_size) == 0) {
      return true;
    } else if (field->v6index &&
               strncmp(reinterpret_cast<char*>(field->entry_index),
                       reinterpret_cast<char*>(needle_for_index),
                       needle_size_for_index) == 0) {
      return true;
    } else {
      return false;
    }
  }
}
