#include "dump.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "util.h"
#include "bocu1.h"

void dump(byte *data, int size)
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

char *inline_dump_str(byte *data, int size)
{
  if (!data) return NULL;
  if (!size) size = strlen((char *)data);

  char *output = (char *)malloc(size*3), *p = output;

  sprintf(p, "%02x", data[0]); p += 2;
  for (int i=1; i<size; ++i) { sprintf(p, " %02x", data[i]); p += 3; }

  return output;
}

char *inline_dump16_str(unichar *data16, int size)
{
  if (!data16) return NULL;

  char *output = (char *)malloc(size*5), *p = output;

  sprintf(p, "%04x", data16[0]); p += 4;
  for (int i=1; i<size; ++i) { sprintf(p, " %04x", data16[i]); p += 5; }

  return output;
}

void inline_dump(byte *data, int size)
{
  if (!data) return;

  char *str = inline_dump_str(data, size);
  printf("%s", str);
  free((void *)str);
}

void inline_dump16(unsigned short *data16, int size)
{
  if (!data16) return;

  char *str = inline_dump16_str(data16, size);
  printf("%s", str);
  free((void *)str);
}

void inline_dump16_in_utf8(unsigned short *data16, int size)
{
  if (!data16) return;

  int dest_size;
  byte *utf8str = encode_utf8(data16, size, dest_size);
  if (!utf8str) return;
  printf("%*s", dest_size, utf8str);
  free((void *)utf8str);
}


void bocu1_dump(byte *bocu1_encoded_data, int size)
{
  if (!size) size = strlen((char *)bocu1_encoded_data);

  int codepoints_len;
  unichar* codepoints = decode_bocu1(bocu1_encoded_data, size, codepoints_len);
  if (codepoints) {
    inline_dump16(codepoints, size);
    free((void *)codepoints);
  }
}

void bocu1_dump_in_utf8(byte *bocu1_encoded_data, int size)
{
  if (!size) size = strlen((char *)bocu1_encoded_data);

  int codepoints_len;
  unichar* codepoints = decode_bocu1(bocu1_encoded_data, size, codepoints_len);
  if (codepoints) {
    inline_dump16_in_utf8(codepoints, size);
    free((void *)codepoints);
  }
}
