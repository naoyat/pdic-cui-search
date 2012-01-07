#include "Criteria.h"
#include <cstdlib>
#include <cstring>
#include <string>

#include "bocu1.h"
#include "util.h"

Criteria::Criteria(unsigned char *needle_utf8, int target_code, bool exact_match)
{
  switch (target_code) {
    case TARGET_BOCU1:
      needle = utf8_to_bocu1(needle_utf8);
      needle_len = strlen((char *)needle);
      break;
    case TARGET_SHIFTJIS:
      needle = utf8_to_sjis(needle_utf8);
      needle_len = strlen((char *)needle);
      //inline_dump(needle, needle_len); newline();
      break;
    default:
      needle_len = strlen((char *)needle_utf8);
      needle = cstr(needle_utf8, needle_len);
      break;
  }
  this->exact_match = exact_match;
}
Criteria::~Criteria()
{
  free((void *)needle);
}

bool
Criteria::match(unsigned char *entry, int entry_len)
{
  if (exact_match) {
    return entry_len == needle_len && strncmp((char *)entry, (char *)needle, needle_len) == 0;
  } else {
    return strncmp((char *)entry, (char *)needle, needle_len) == 0;
  }
}
