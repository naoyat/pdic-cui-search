#ifndef UTIL_H
#define UTIL_H

#include <cstdio>
#include <cstring>

#include "types.h"

inline int s8val(byte *data) { return *(char *)data; }
inline int s16val(byte *data) { return *(short *)data; }
inline int s32val(byte *data) { return *(long *)data; }
inline long long s64val(byte *data) { return *(long long *)data; }
inline unsigned int u8val(byte *data) { return *(unsigned char *)data; }
inline unsigned int u16val(byte *data) { return *(unsigned short *)data; }
inline unsigned int u32val(byte *data) { return *(unsigned long *)data; }
inline unsigned long long u64val(byte *data) { return *(unsigned long long *)data; }

void *clone(void *data, size_t size, bool gc=true);
byte *clone_cstr(byte *data, int length=0, bool gc=true);
void free_cloned_buffer(void *ptr);
void free_all_cloned_buffers();

// byte版 str(n)cmp
int bstrcmp(byte *s1, byte *s2, int minimum_charcode=0x20);
int bstrncmp(byte *s1, byte *s2, size_t n, int minimum_charcode=0x20);
int bstrcicmp(byte *s1, byte *s2, int minimum_charcode=0x20);

// ポインタ版 bstr(n)cmp
int pbstrcmp(const void *s1, const void *s2);
int pbstrncmp(const void *s1, const void *s2, size_t n);

byte *strhead(byte *ptr);

inline bool is_not_empty(byte *ptr) { return ptr && ptr[0]; }
inline bool is_empty(byte *ptr) { return (!ptr) || !ptr[0]; }

#endif
