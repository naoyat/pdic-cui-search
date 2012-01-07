#ifndef __CRITERIA_H
#define __CRITERIA_H

#define TARGET_ASCII     0
#define TARGET_LATIN1    1
#define TARGET_SHIFTJIS  7
#define TARGET_UTF8      8
#define TARGET_BOCU1     9

class Criteria
{
  unsigned char *needle;
  int needle_len;

  bool exact_match;

public:
  Criteria(unsigned char *needle_utf8, int target_code, bool exact_match);
  ~Criteria();
  
public:
  bool match(unsigned char *entry, int entry_len);
};

#endif

