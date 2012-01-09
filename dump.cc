#include "dump.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "util.h"
#include "bocu1.h"

void dump(unsigned char *data, int size)
{
  if (!data) return;
  if (!size) size = strlen((char *)data);

  for (int i=0; i<size; i+=16) {
    printf("%08x:", i);
    for (int j=0; j<16; ++j) printf(" %02x", data[i+j]);

    printf("  ");
    for (int j=0; j<16; ++j) {
      int ch = data[i+j];
      if (0x20 <= ch && ch <= 0x7e) putchar(ch);
      else putchar('.');
    }

    printf("\n");
  }
}

void inline_dump(unsigned char *data, int size)
{
  if (!data) return;
  if (!size) size = strlen((char *)data);

  printf("%02x", data[0]);
  for (int i=1; i<size; ++i) printf(" %02x", data[i]);
}

void inline_dump16(unsigned short *data16, int size)
{
  if (!data16) return;

  printf("%04x", data16[0]);
  for (int i=1; i<size; ++i) printf(" %04x", data16[i]);
}

void inline_dump16_in_utf8(unsigned short *data16, int size)
{
  if (!data16) return;

  int dest_size;
  unsigned char *utf8str = encode_utf8(data16, size, dest_size);
  if (!utf8str) return;
  printf("%*s", dest_size, utf8str);
  free((void *)utf8str);
}


void bocu1_dump(unsigned char *bocu1_encoded_data, int size)
{
  if (!size) size = strlen((char *)bocu1_encoded_data);

  int codepoints_len;
  unichar* codepoints = decode_bocu1(bocu1_encoded_data, size, codepoints_len);
  if (codepoints) {
    inline_dump16(codepoints, size);
    free((void *)codepoints);
  }
}

void bocu1_dump_in_utf8(unsigned char *bocu1_encoded_data, int size)
{
  if (!size) size = strlen((char *)bocu1_encoded_data);

  int codepoints_len;
  unichar* codepoints = decode_bocu1(bocu1_encoded_data, size, codepoints_len);
  if (codepoints) {
    inline_dump16_in_utf8(codepoints, size);
    free((void *)codepoints);
  }
}
