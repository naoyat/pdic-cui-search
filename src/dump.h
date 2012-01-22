// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SRC_DUMP_H_
#define SRC_DUMP_H_

#include <stdio.h>

#include "./types.h"


char* inline_dump_str(byte* data, int size = 0);
char* inline_dump16_str(unichar* data, int size);

void dump(byte* data, int size = 0);
void inline_dump(byte* data, int size = 0);
void inline_dump16(unichar* data, int size);
void inline_dump16_in_utf8(unichar* data, int size);

void bocu1_dump(byte* bocu1_encoded_data, int size = 0);
void bocu1_dump_in_utf8(byte* bocu1_encoded_data, int size = 0);

#endif  // SRC_DUMP_H_
