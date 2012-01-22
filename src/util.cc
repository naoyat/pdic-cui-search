#include "util.h"

#include <set>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "util_stl.h"


std::set<void*> clone_ptrs;

void free_cloned_buffer(void *ptr)
{
  if (found(clone_ptrs,ptr)) {
    free(ptr);
    clone_ptrs.erase(ptr);
  }
}
void free_all_cloned_buffers()
{
  traverse(clone_ptrs,ptr) free(*ptr);
  clone_ptrs.clear();
}

void *clone(void *data, size_t size, bool gc)
{
  if (!data) return NULL;
  void *buf = malloc(size);
  memcpy(buf, data, size);
  if (gc) clone_ptrs.insert(buf);
  return buf;
}

byte *clone_cstr(byte *data, int length, bool gc)
{
  if (!data) return (byte *)NULL;
  if (!data[0] && gc) return (byte *)"";
  if (!length) length = strlen((char *)data);

  void *newstr = clone((void *)data, length+1, gc);
  ((byte *)newstr)[length] = 0;

  return (byte *)newstr;
}

int bstrcmp(byte *s1, byte *s2, int minimum_charcode)
{
  if (!s1) return -1;
  if (!s2) return 1;
  //printf("bstrcmp(\"%s\", \"%s\")...\n", (const char*)s1, (const char*)s2);
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
  for (uint i=0; i<n; ++i) {
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

int bstrcicmp(byte *s1, byte *s2, int minimum_charcode)
{
  for (int i=0; ; ++i) {
    int c1 = tolower(s1[i]), c2 = tolower(s2[i]);
    if (s1[i] < minimum_charcode) {
      if (s2[i] < minimum_charcode) return 0;
      else                          return c1 > c2 ? 1 : -1;
    } else if (s2[i] < minimum_charcode) {
      return 1;
    } else {
      if (c1 != c2) {
        return c1 > c2 ? 1 : -1;
      }
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
