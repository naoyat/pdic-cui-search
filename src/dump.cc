// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./dump.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./bocu1.h"
#include "./utf8.h"
#include "./util.h"

const char *hexstr = "0123456789abcdef";

void dump(byte* data, int size) {
  if (!data) return;
  if (!size) size = strlen(reinterpret_cast<char*>(data));

  for (int i = 0; i < size; i += 16) {
    printf("%08x:", i);
    for (int j = 0; j < 16; ++j) printf(" %02x", data[i+j]);

    printf("  ");
    for (int j = 0; j < 16; ++j) {
      int ch = data[i+j];
      if (0x20 <= ch && ch <= 0x7e)
        putchar(ch);
      else
        putchar('.');
    }

    printf("\n");
  }
}

char* inline_dump_str(byte* data, int size) {
  if (!data) return NULL;
  if (!size) size = strlen(reinterpret_cast<char*>(data));

  char* output = static_cast<char*>(malloc(size * 3)), *p = output;

  *(p++) = hexstr[data[0] >> 4];
  *(p++) = hexstr[data[0] & 0xf];
  // snprintf(p, 2, "%02x", data[0]); p += 2;
  for (int i = 1; i < size; ++i) {
    *(p++) = ' ';
    *(p++) = hexstr[data[i] >> 4];
    *(p++) = hexstr[data[i] & 0x0f];
    // snprintf(p, 3, " %02x", data[i]); p += 3;
  }

  return output;
}

char* inline_dump16_str(unichar* data16, int size) {
  if (!data16) return NULL;

  char* output = static_cast<char*>(malloc(size * 5)), *p = output;

  *(p++) = hexstr[(data16[0] >> 12) & 0x0f];
  *(p++) = hexstr[(data16[0] >> 8) & 0x0f];
  *(p++) = hexstr[(data16[0] >> 4) & 0x0f];
  *(p++) = hexstr[data16[0] & 0x0f];
  // snprintf(p, 4, "%04x", data16[0]); p += 4;
  for (int i = 1; i < size; ++i) {
    *(p++) = ' ';
    *(p++) = hexstr[(data16[i] >> 12) & 0x0f];
    *(p++) = hexstr[(data16[i] >> 8) & 0x0f];
    *(p++) = hexstr[(data16[i] >> 4) & 0x0f];
    *(p++) = hexstr[data16[i] & 0x0f];
    // snprintf(p, 5, " %04x", data16[i]); p += 5;
  }

  return output;
}

void inline_dump(byte* data, int size) {
  if (!data) return;

  char* str = inline_dump_str(data, size);
  printf("%s", str);
  free(static_cast<void*>(str));
}

void inline_dump16(unichar *data16, int size) {
  if (!data16) return;

  char* str = inline_dump16_str(data16, size);
  printf("%s", str);
  free(static_cast<void*>(str));
}

void inline_dump16_in_utf8(unichar *data16, int size) {
  if (!data16) return;

  int dest_size;
  byte* utf8str = encode_utf8(data16, size, &dest_size);
  if (!utf8str) return;
  printf("%*s", dest_size, utf8str);
  free(static_cast<void*>(utf8str));
}

void bocu1_dump(byte* bocu1_encoded_data, int size) {
  if (!size) size = strlen(reinterpret_cast<char*>(bocu1_encoded_data));

  int codepoints_len;
  unichar* codepoints = decode_bocu1(bocu1_encoded_data, size, &codepoints_len);
  if (codepoints) {
    inline_dump16(codepoints, size);
    free(static_cast<void*>(codepoints));
  }
}

void bocu1_dump_in_utf8(byte* bocu1_encoded_data, int size) {
  if (!size) size = strlen(reinterpret_cast<char*>(bocu1_encoded_data));

  int codepoints_len;
  unichar* codepoints = decode_bocu1(bocu1_encoded_data, size, &codepoints_len);
  if (codepoints) {
    inline_dump16_in_utf8(codepoints, size);
    free(static_cast<void*>(codepoints));
  }
}
