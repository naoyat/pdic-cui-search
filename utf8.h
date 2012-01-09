#ifndef __UTF8_H
#define __UTF8_H

#include <cstdio>

typedef unsigned short unichar;

/*
 * Unicode & UTF-8
 */

// unicode
bool surrogate(int codepoint, int *upper, int *lower);
int unsurrogate(int upper, int lower);

// codepoints <=> utf8
unsigned char *encode_utf8(unichar *src_codepoint, int src_size, int& dest_size);
unichar *decode_utf8(unsigned char *src_utf8, int src_size, int& dest_size);

// sjis <=> utf8 (using iconv)
char *_iconv(const char *src, size_t src_size, const char *src_code, char *dest, size_t dest_size, const char *dest_code);

unsigned char *sjis_to_utf8(unsigned char *src_sjis, int size=0);
unsigned char *utf8_to_sjis(unsigned char *src_utf8, int size=0);


#endif
