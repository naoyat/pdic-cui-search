#ifndef __UTIL_H_
#define __UTIL_H_

#include <cstdio>

inline int byteval(unsigned char *data) { return *(char *)data; }
inline int shortval(unsigned char *data) { return *(short *)data; }
inline int longval(unsigned char *data) { return *(long *)data; }
inline long long longlongval(unsigned char *data) { return *(long long *)data; }
inline unsigned int ubyteval(unsigned char *data) { return *(unsigned char *)data; }
inline unsigned int ushortval(unsigned char *data) { return *(unsigned short *)data; }
inline unsigned int ulongval(unsigned char *data) { return *(unsigned long *)data; }
inline unsigned long long ulonglongval(unsigned char *data) { return *(unsigned long long *)data; }

int bsearch_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle);
int bsearch2_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle, bool exact_match, int& lo, int& hi);

char *indent(char *spacer, char *str);

unsigned char *cstr(unsigned char *data, int length=0);

#endif
