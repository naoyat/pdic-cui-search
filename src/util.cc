#include "util.h"

#include <cstdlib>
#include <cstring>

#include "types.h"

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

void *clone(void *data, size_t size)
{
  void *buf = malloc(size);
  memcpy(buf, data, size);
  return buf;
}

byte *cstr(byte *data, int length)
{
  if (!length) length = strlen((char *)data);

  void *newstr = clone((void *)data, length+1);
  data[length] = 0;

  return (byte *)newstr;
}

int bstrcmp(byte *s1, byte *s2, int minimum_charcode)
{
  for (int i=0; ; ++i) {
    if (s1[i] < minimum_charcode) {
      if (s2[i] < minimum_charcode) return 0;
      else                          return s1[i] > s2[i] ? 1 : -1;
    } else if (s2[i] < minimum_charcode) {
      return 1;
    } else if (s1[i] != s2[i]) {
      return s1[i] > s2[i] ? 1 : -1;
    }
  }
  return 0;
  /*
  byte *p1 = s1, *p2 = s2;
  while ((*p1 >= minimum_charcode) && (*p1 == *p2)) { ++p1; ++p2; }
  if (*p1 > *p2) return 1;
  else if (*p1 < *p2) return -1;
  else return 0;
  */
}
int bstrncmp(byte *s1, byte *s2, size_t n, int minimum_charcode)
{
  for (int i=0; i<n; ++i) {
    if (s1[i] < minimum_charcode) {
      if (s2[i] < minimum_charcode) return 0;
      else                          return s1[i] > s2[i] ? 1 : -1;
    } else if (s2[i] < minimum_charcode) {
      return 1;
    } else if (s1[i] != s2[i]) {
      return s1[i] > s2[i] ? 1 : -1;
    }
  }
  return 0;
}


int pbstrcmp(const void *s1, const void *s2)
{
  return bstrcmp(*(byte **)s1, *(byte **)s2);
}
int pbsrncmp(const void *s1, const void *s2, size_t n)
{
  return bstrncmp(*(byte **)s1, *(byte **)s2, n);
}

byte *strhead(byte *ptr)
{
  byte *p = ptr;
  while (*p) --p;
  return p+1;
}
