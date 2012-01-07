#ifndef __UTIL_H_
#define __UTIL_H_

#include <cstdio>

typedef unsigned short unichar;

void dump(unsigned char *data, int size=0);
void inline_dump(unsigned char *data, int size=0);
void inline_dump16(unsigned short *data, int size);
void inline_dump16_in_utf8(unsigned short *data, int size);

inline int byteval(unsigned char *data) { return *(char *)data; }
inline int shortval(unsigned char *data) { return *(short *)data; }
inline int longval(unsigned char *data) { return *(long *)data; }
inline long long longlongval(unsigned char *data) { return *(long long *)data; }
inline unsigned int ubyteval(unsigned char *data) { return *(unsigned char *)data; }
inline unsigned int ushortval(unsigned char *data) { return *(unsigned short *)data; }
inline unsigned int ulongval(unsigned char *data) { return *(unsigned long *)data; }
inline unsigned long long ulonglongval(unsigned char *data) { return *(unsigned long long *)data; }

unsigned char *encode_utf8(unichar *src_codepoint, int src_size, int& dest_size);
unichar *decode_utf8(unsigned char *src_utf8, int src_size, int& dest_size);

int bsearch_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle);
int bsearch2_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle, bool exact_match, int& lo, int& hi);

inline void newline() { putchar('\n'); }

char *indent(char *spacer, char *str);

char *_iconv(const char *src, size_t src_size, const char *src_code, char *dest, size_t dest_size, const char *dest_code);

unsigned char *sjis_to_utf8(unsigned char *src_sjis, int size=0);
unsigned char *utf8_to_sjis(unsigned char *src_utf8, int size=0);

unsigned char *cstr(unsigned char *data, int length=0);

#endif
