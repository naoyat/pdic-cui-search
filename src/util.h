#ifndef UTIL_H
#define UTIL_H

#include <cstdio>
#include <cstring>

inline int byteval(unsigned char *data) { return *(char *)data; }
inline int shortval(unsigned char *data) { return *(short *)data; }
inline int longval(unsigned char *data) { return *(long *)data; }
inline long long longlongval(unsigned char *data) { return *(long long *)data; }
inline unsigned int ubyteval(unsigned char *data) { return *(unsigned char *)data; }
inline unsigned int ushortval(unsigned char *data) { return *(unsigned short *)data; }
inline unsigned int ulongval(unsigned char *data) { return *(unsigned long *)data; }
inline unsigned long long ulonglongval(unsigned char *data) { return *(unsigned long long *)data; }

char *indent(char *spacer, char *str);

void *clone(void *data, size_t size);
unsigned char *cstr(unsigned char *data, int length=0);

int ustrcmp(unsigned char *s1, unsigned char *s2);
inline int ustrncmp(unsigned char *s1, unsigned char *s2, size_t n) { return memcmp(s1,s2,n); }

#endif