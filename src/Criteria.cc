#include "Criteria.h"

#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <re2/re2.h>

#include "bocu1.h"
#include "util.h"
#include "dump.h"
#include "types.h"
#include "charcode.h"
#include "PDICHeader.h"
#include "PDICDatafield.h"
#include "PDICIndex.h" // string_for_index()

Criteria::Criteria(byte *needle_utf8, int target_charcode, bool exact_match) :
    re2_pattern(std::string("(?i)")+(char*)needle_utf8)
{
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
  //free_cloned_buffer(needle_utf8);
  free((void *)needle);
  if (needle_for_index) free((void *)needle_for_index);
  if (needle_for_index_utf8) free((void *)needle_for_index_utf8);
}

bool
Criteria::match(PDICDatafield *field)
{
  /*
  printf("%%match(needle="); bocu1_dump_in_utf8( needle );
  printf(", [");
  bocu1_dump_in_utf8( field->entry_index );
  printf("|");
  bocu1_dump_in_utf8( field->entry_word );
  printf("])\n");
  */
  if (exact_match) {
    if (field->entry_word_size == needle_size
        && strncmp((char *)field->entry_word, (char *)needle, needle_size) == 0)
      return true;
    else if (field->v6index &&
             (field->entry_index_size == needle_size_for_index
              && strncmp((char *)field->entry_index, (char *)needle_for_index, needle_size_for_index) == 0))
      return true;
      else return false;
  } else {
    if (strncmp((char *)field->entry_word, (char *)needle, needle_size) == 0) return true;
    else if (field->v6index &&
             strncmp((char *)field->entry_index, (char *)needle_for_index, needle_size_for_index) == 0 )
      return true;
    else return false;
  }
}
