// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SRC_UTF8_H_
#define SRC_UTF8_H_

#include <stdio.h>

#include "./types.h"


/*
 * Unicode & UTF-8
 */

// unicode
bool surrogate(int codepoint, int* upper, int* lower);
int unsurrogate(int upper, int lower);

// codepoints <=> utf8
byte *encode_utf8(unichar* src_codepoint, int src_size, int* dest_size);
unichar *decode_utf8(byte* src_utf8, int src_size, int* dest_length);

// sjis <=> utf8 (using iconv)
char *_iconv(const char* src, size_t src_size, const char* src_code,
             char* dest, size_t dest_size, const char* dest_code);

byte *sjis_to_utf8(byte* src_sjis, int size = 0);
byte *utf8_to_sjis(byte* src_utf8, int size = 0);

#endif  // SRC_UTF8_H_
