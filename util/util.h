// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util/types.h"

inline byte* BYTE(char *str) {
  return reinterpret_cast<byte*>(str);
}

inline int s8val(byte* data) {
  return *reinterpret_cast<int8_t*>(data);
}
inline int s16val(byte* data) {
  return *reinterpret_cast<int16_t*>(data);
}
inline int s32val(byte* data) {
  return *reinterpret_cast<int32_t*>(data);
}
inline int64_t s64val(byte* data) {
  return *reinterpret_cast<int64_t*>(data);
}
inline uint u8val(byte* data) {
  return *reinterpret_cast<uint8_t*>(data);
}
inline uint u16val(byte* data) {
  return *reinterpret_cast<uint16_t*>(data);
}
inline uint u32val(byte* data) {
  /*
  printf("u32val(%02x %02x %02x %02x) -> %lld\n",
         data[0], data[1], data[2], data[3],
         *reinterpret_cast<uint32_t*>(data));
         */
  return *reinterpret_cast<uint32_t*>(data);
}
inline uint64_t u64val(byte* data) {
  return *reinterpret_cast<uint64_t*>(data);
}

void* clone(void* data, size_t size, bool gc = true);
byte* clone_cstr(byte* data, int length = 0, bool gc = true);
void free_cloned_buffer(void* ptr);
void free_all_cloned_buffers();

// byte版 str(n)cmp
int bstrcmp(byte* s1, byte* s2, int minimum_charcode = 0x20);
int bstrncmp(byte* s1, byte* s2, size_t n, int minimum_charcode = 0x20);
int bstrcicmp(byte* s1, byte* s2, int minimum_charcode = 0x20);

// ポインタ版 bstr(n)cmp
int pbstrcmp(const void* s1, const void* s2);
int pbstrncmp(const void* s1, const void* s2, size_t n);

byte* strhead(byte* ptr);

inline bool is_not_empty(byte* ptr) { return ptr && ptr[0]; }
inline bool is_empty(byte* ptr) { return (!ptr) || !ptr[0]; }

#endif  // UTIL_UTIL_H_
