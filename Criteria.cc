#include "Criteria.h"
#include <cstdlib>
#include <cstring>

#include "bocu1.h"


Criteria::Criteria(unsigned char *needle_utf8, bool exact_match)
{
  needle_bocu1 = utf8_to_bocu1(needle_utf8);
  needle_len = strlen((char *)needle_bocu1);
  this->exact_match = exact_match;
}
Criteria::~Criteria()
{
  free((void *)needle_bocu1);
}

bool
Criteria::match(unsigned char *entry, int entry_len)
{
  if (exact_match) {
    return entry_len == needle_len && strncmp((char *)entry, (char *)needle_bocu1, needle_len) == 0;
  } else {
    return strncmp((char *)entry, (char *)needle_bocu1, needle_len) == 0;
  }
}

