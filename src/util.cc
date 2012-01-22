// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./util.h"

#include <stdlib.h>
#include <string.h>

#include <set>

#include "./stlutil.h"
#include "./types.h"


std::set<void*> clone_ptrs;

void free_cloned_buffer(void* ptr) {
  if (found(clone_ptrs, ptr)) {
    free(ptr);
    clone_ptrs.erase(ptr);
  }
}

void free_all_cloned_buffers() {
  traverse(clone_ptrs, ptr) free(*ptr);
  clone_ptrs.clear();
}

void* clone(void* data, size_t size, bool gc) {
  if (!data) return NULL;
  void *buf = malloc(size);
  memcpy(buf, data, size);
  if (gc) clone_ptrs.insert(buf);
  return buf;
}

byte* clone_cstr(byte* data, int length, bool gc) {
  if (!data) return static_cast<byte*>NULL;
  if (!data[0] && gc) return static_cast<byte*>"";
  if (!length) length = strlen(static_cast<char*>data);

  byte* newstr = static_cast<byte*>clone(static_cast<void*>data, length+1, gc);
  newstr[length] = 0;

  return newstr;
}

int bstrcmp(byte *s1, byte *s2, int minimum_charcode) {
  if (!s1) return -1;
  if (!s2) return 1;

  for (int i = 0; ; ++i) {
    if (s1[i] < minimum_charcode) {
      if (s2[i] < minimum_charcode)
        return 0;
      else
        return s1[i] > s2[i] ? 1 : -1;
    } else if (s2[i] < minimum_charcode) {
      return 1;
    } else if (s1[i] != s2[i]) {
      return s1[i] > s2[i] ? 1 : -1;
    }
  }

  return 0;
}

int bstrncmp(byte *s1, byte *s2, size_t n, int minimum_charcode) {
  for (uint i = 0; i < n; ++i) {
    if (s1[i] < minimum_charcode) {
      if (s2[i] < minimum_charcode)
        return 0;
      else
        return s1[i] > s2[i] ? 1 : -1;
    } else if (s2[i] < minimum_charcode) {
      return 1;
    } else if (s1[i] != s2[i]) {
      return s1[i] > s2[i] ? 1 : -1;
    }
  }
  return 0;
}

int bstrcicmp(byte *s1, byte *s2, int minimum_charcode) {
  for (int i = 0; ; ++i) {
    int c1 = tolower(s1[i]), c2 = tolower(s2[i]);
    if (s1[i] < minimum_charcode) {
      if (s2[i] < minimum_charcode)
        return 0;
      else
        return c1 > c2 ? 1 : -1;
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

int pbstrcmp(const void *s1, const void *s2) {
  return bstrcmp(*static_cast<byte**>s1, *static_cast<byte**>s2);
}

int pbsrncmp(const void *s1, const void *s2, size_t n) {
  return bstrncmp(*static_cast<byte**>s1, *static_cast<byte**>s2, n);
}

byte *strhead(byte *ptr) {
  byte *p = ptr;
  while (*p) --p;

  return p+1;
}
