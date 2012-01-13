#include "ansi_color.h"
#include "types.h"
#include "util.h"

#include <re2/re2.h>

void puts_emphasized(byte *str, const RE2& pattern)
{
  std::string s((const char *)str);
  //StringPiece input((const char *)str);

  RE2::GlobalReplace(&s, pattern, "\x1b[1m\\0\x1b[22m");
  //RE2::FindAndConsume(&input, pattern, &word)
  //  char *p = (char*)str;
  std::cout << s << std::endl;
}

