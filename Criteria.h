#ifndef __CRITERIA_H
#define __CRITERIA_H

class Criteria
{
  unsigned char *needle_bocu1;
  int needle_len;

  bool exact_match;

public:
  Criteria(unsigned char *needle_utf8, bool exact_match);
  ~Criteria();
  
public:
  bool match(unsigned char *entry, int entry_len);
};

#endif

