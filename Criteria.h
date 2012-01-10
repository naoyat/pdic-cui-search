#ifndef CRITERIA_H
#define CRITERIA_H

#include "PDICDatafield.h"

class Criteria
{
public:
  unsigned char *needle;
  int            needle_size;
  bool           exact_match;

public:
  Criteria(unsigned char *needle_utf8, int target_charcode, bool exact_match);
  ~Criteria();
  
public:
  //bool match(unsigned char *entry, int entry_len);
  bool match(PDICDatafield *datafield);
};

#endif

