#include "Criteria.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include "bocu1.h"
#include "util.h"
#include "dump.h"
#include "types.h"
#include "charcode.h"
#include "PDICDatafield.h"

Criteria::Criteria(byte *needle_utf8, int target_charcode, bool exact_match)
{
  switch (target_charcode) {
    case CHARCODE_BOCU1:
      needle = utf8_to_bocu1(needle_utf8);
      needle_size = strlen((char *)needle);
      break;
    case CHARCODE_SHIFTJIS:
      needle = utf8_to_sjis(needle_utf8);
      needle_size = strlen((char *)needle);
      break;
    default:
      needle_size = strlen((char *)needle_utf8);
      needle = clone_cstr(needle_utf8, needle_size, false);
      break;
  }
  this->exact_match = exact_match;
}
Criteria::~Criteria()
{
  free((void *)needle);
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
    /*
    //    if (field->entry_word_size == needle_size
    //&& strncmp((char *)field->entry_word, (char *)needle, needle_size) == 0)
      printf("match(): %s == %s\n",
           inline_dump_str(field->entry_word, field->entry_word_size),
           inline_dump_str(needle, needle_size) );
    */
    return (field->entry_index_size == needle_size
            && strncmp((char *)field->entry_index, (char *)needle, needle_size) == 0)
        || (field->entry_word_size == needle_size
            && strncmp((char *)field->entry_word, (char *)needle, needle_size) == 0);
  } else {
    return (strncmp((char *)field->entry_index, (char *)needle, needle_size) == 0)
        || (strncmp((char *)field->entry_word, (char *)needle, needle_size) == 0);
  }
}
