#include "Criteria.h"

#include <stdlib.h>
#include <string.h>

#include <sstream>
#include <string>

#include <re2/re2.h>

#include "bocu1.h"
#include "charcode.h"
#include "dump.h"
#include "PDICDatafield.h"
#include "PDICHeader.h"
#include "PDICIndex.h" // string_for_index()
#include "types.h"
#include "utf8.h"
#include "util.h"


Criteria::Criteria(byte *needle_utf8, int target_charcode, bool exact_match)
{
  this->re2_pattern = new RE2(std::string("(?i)") + (char*)needle_utf8);

  this->needle_utf8 = needle_utf8;
  this->needle_for_index_utf8 = (byte*)NULL;

  switch (target_charcode) {
    case CHARCODE_BOCU1: {
      needle = utf8_to_bocu1(needle_utf8);
      needle_size = strlen((char *)needle);

      this->needle_for_index_utf8 = string_for_index(needle_utf8);
      needle_for_index = utf8_to_bocu1(this->needle_for_index_utf8);
      needle_size_for_index = strlen((char *)needle_for_index);
    }
      break;
    case CHARCODE_SHIFTJIS:
      needle = utf8_to_sjis(needle_utf8);
      needle_size = strlen((char *)needle);
      needle_for_index = (byte*)NULL;
      needle_size_for_index = 0;
      break;
    default:
      needle_size = strlen((char *)needle_utf8);
      needle_for_index = (byte*)NULL;
      needle_size_for_index = 0;
      needle = clone_cstr(needle_utf8, needle_size, false);
      break;
  }
  this->exact_match = exact_match;
}

Criteria::~Criteria()
{
  free((void *)needle);
  if (needle_for_index) free((void *)needle_for_index);
  if (needle_for_index_utf8) free((void *)needle_for_index_utf8);
  delete re2_pattern;
}

bool Criteria::match(PDICDatafield *field)
{
  if (exact_match) {
    if (field->entry_word_size == needle_size
        && strncmp((char *)field->entry_word, (char *)needle, needle_size) == 0) {
      return true;
    } else if (field->v6index &&
             (field->entry_index_size == needle_size_for_index
              && strncmp((char *)field->entry_index,
                         (char *)needle_for_index, needle_size_for_index) == 0)) {
      return true;
    } else {
      return false;
    }
  } else {
    if (strncmp((char *)field->entry_word, (char *)needle, needle_size) == 0) {
      return true;
    } else if (field->v6index &&
               strncmp((char *)field->entry_index,
                       (char *)needle_for_index, needle_size_for_index) == 0) {
      return true;
    } else {
      return false;
    }
  }
}
