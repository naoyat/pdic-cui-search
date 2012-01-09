#include "util.h"

#include <cstdlib>
#include <cstring>

char *indent(char *spacer, char *src)
{
  int len_before = strlen(src);
  int len_spacer = strlen(spacer);
  // pass1
  int nl = 0;
  for (char *p=src; *p; ++p) {
    if (*p == '\n') nl++;
  }
  int len_after = len_before + (1+nl)*len_spacer;
  char *indented = (char *)malloc(len_after + 1); indented[len_after] = 0;

  strcpy(indented, spacer); char *dest = indented + len_spacer;
  for (char *p=src; *p; ++p) {
    *(dest++) = *p;
    if (*p == '\n') { strcpy(dest, spacer); dest += len_spacer; }
  }
  return indented;
}

unsigned char *cstr(unsigned char *data, int length)
{
  if (!length) length = strlen((char *)data);

  unsigned char *buf = (unsigned char *)malloc(length + 1);
  memcpy(buf, data, length); data[length] = 0;
  return buf;
}
