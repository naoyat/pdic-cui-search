// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDICCUISEARCH_BOCU1_H_
#define PDICCUISEARCH_BOCU1_H_

#include "types.h"


// codepoints <=> bocu1
byte *encode_bocu1(unichar *src_codepoint, int src_len, int& dest_size);
unichar *decode_bocu1(byte *src_bocu1, int src_size, int& dest_size);

// utf8 <=> bocu1 (via codepoints), returns NULL-ending string
byte *utf8_to_bocu1(byte *src_utf8, int src_size=0);//, int& dest_size);
byte *bocu1_to_utf8(byte *src_bocu1, int src_size=0);//, int& dest_size);

void bocu1_check(byte *bocu1_encoded_data, int size=0);

#endif // PDICCUISEARCH_BOCU1_H_
