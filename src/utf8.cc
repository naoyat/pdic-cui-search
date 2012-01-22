#include "utf8.h"

#include <iconv.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"


bool surrogate(int codepoint, int *upper, int *lower)
{
  if (0x10000 <= codepoint && codepoint < 0x110000) {
    int x = codepoint - 65536;
    *upper = 0xd800 + (x / 1024);
    *lower = 0xdc00 + (x % 1024);
    //printf("surrogate(%x) -> %04x:%04x\n", *upper, *lower);
    return true;
  } else {
    *upper = *lower = -1;
    return false;
  }
}

int unsurrogate(int upper, int lower)
{
  if (upper < 0xd800 || 0xdbff < upper || lower < 0xdc00 || 0xdfff < lower) return -1;
  return 0x10000 + (((upper - 0xd800) << 10) | (lower - 0xdc00));
}


byte *encode_utf8(unichar *src_codepoint, int src_size, int& dest_size)
{
  dest_size = 0;
  for (int i=0; i<src_size; i++) {
    int codepoint = src_codepoint[i];
    if (codepoint == 0) {
      break;
    } else if (codepoint <= 0x7f) {
      dest_size += 1;
    } else if (codepoint <= 0x07ff) {
      dest_size += 2;
    } else /* if (src_codepoint[i] <= 0xffff) */ {
      if (0xd800 <= codepoint && codepoint <= 0xdbff) {
        //int upper = codepoint - 0xd800;
        //int lower = src_codepoint[++i] - 0xdc00;
        dest_size += 4;
      } else {
        dest_size += 3;
      }
    }
  }

  byte *dest = (byte *)malloc(dest_size + 1);
  if (!dest) return NULL;

  int j=0;
  for (int i=0; i<src_size; i++) {
    int codepoint = src_codepoint[i];
    if (codepoint == 0) {
      break;
    } else if (codepoint <= 0x007f) {
      dest[j++] = codepoint;
    } else if (codepoint <= 0x07ff) {
      dest[j++] = 0xc0 | (codepoint >> 6);
      dest[j++] = 0x80 | (codepoint & 0x3f);
    } else { //if (codepoint <= 0xffff) {
      if (0xd800 <= codepoint && codepoint <= 0xdbff) {
        int next_code = src_codepoint[++i];
        if (0xdc00 <= next_code && next_code <= 0xdfff) {
          int upper = codepoint - 0xd800;
          int lower = next_code - 0xdc00;
          codepoint = 0x10000 + ((upper << 10) | lower);
          dest[j++] = 0xf0 | (codepoint >> 18);
          dest[j++] = 0x80 | ((codepoint >> 12) & 0x3f);
          dest[j++] = 0x80 | ((codepoint >> 6) & 0x3f);
          dest[j++] = 0x80 | (codepoint & 0x3f);
        } else {
          // illegal
          --i;
          dest[j++] = 0xe0 | (codepoint >> 12);
          dest[j++] = 0x80 | ((codepoint >> 6) & 0x3f);
          dest[j++] = 0x80 | (codepoint & 0x3f);
        }
      } else {
        dest[j++] = 0xe0 | (codepoint >> 12);
        dest[j++] = 0x80 | ((codepoint >> 6) & 0x3f);
        dest[j++] = 0x80 | (codepoint & 0x3f);
      }
    }
  }
  dest[j] = 0;

  return dest;
}

unichar *decode_utf8(byte *src_utf8, int src_size, int& dest_length)
{
  dest_length = 0;

  unichar* dest = (unichar *)malloc(sizeof(unichar)*(src_size+1));
  if (!dest) return NULL;

  for (int i=0; i<src_size; ) {
    int a = src_utf8[i++];
    int codepoint;
    if (a <= 0x7f) {
      // U+00 .. U+7F
      codepoint = a;
    } else if (a <= 0xdf) {
      // U+0080 .. U+07FF
      a &= 0x1f;
      int b = src_utf8[i++] & 0x3f;
      codepoint = (a << 6) | b;
    } else if (a <= 0xef) {
      // U+0800 .. U+FFFF
      a &= 0x0f;
      int b = src_utf8[i++] & 0x3f;
      int c = src_utf8[i++] & 0x3f;
      codepoint = (a << 12) | (b << 6) | c;
    } else if (a <= 0xf7) {
      // U+01000 .. U+1FFFFF
      a &= 0x07;
      int b = src_utf8[i++] & 0x3f;
      int c = src_utf8[i++] & 0x3f;
      int d = src_utf8[i++] & 0x3f;
      codepoint = (a << 18) | (b << 12) | (c << 6) | d;
    } else if (a <= 0xfb) {
      // U+00200000 .. U+03FFFFFF
      a &= 0x03;
      int b = src_utf8[i++] & 0x3f;
      int c = src_utf8[i++] & 0x3f;
      int d = src_utf8[i++] & 0x3f;
      int e = src_utf8[i++] & 0x3f;
      codepoint = (a << 24) | (b << 18) | (c << 12) | (d << 6) | e;
    } else if (a <= 0xfd) {
      // U+04000000 .. U+7FFFFFFF
      a &= 0x01;
      int b = src_utf8[i++] & 0x3f;
      int c = src_utf8[i++] & 0x3f;
      int d = src_utf8[i++] & 0x3f;
      int e = src_utf8[i++] & 0x3f;
      int f = src_utf8[i++] & 0x3f;
      codepoint = (a << 30) | (b << 24) | (c << 18) | (d << 12) | (e << 6) | f;
    } else {
      // BOM, EOM
    }

    if (codepoint < 0x10000) {
      dest[dest_length++] = codepoint;
    } else if (codepoint < 0x110000) {
      int upper, lower;
      surrogate(codepoint, &upper, &lower);
      //printf("{%x -> %04x:%04x}", codepoint, upper, lower);
      dest[dest_length++] = upper;
      dest[dest_length++] = lower;
    } else {
      // simply ignore
    }
  }

  dest[dest_length] = 0;

  unichar *newp = (unichar *)realloc((void *)dest, sizeof(unichar)*(dest_length+1));
  return newp ? newp : dest;
}

char *_iconv(const char *src, size_t src_size, const char *src_code, char *dest, size_t dest_size, const char *dest_code)
{
  iconv_t icd = iconv_open(dest_code, src_code);

  size_t n_src = src_size, n_dest = dest_size;
  char *p_src = (char *)src, *p_dest = (char *)dest;
  while (n_src > 0) {
    iconv(icd, &p_src, &n_src, &p_dest, &n_dest);
  }
  *p_dest = 0;

  int actual_size = (int)(p_dest - (char *)dest);

  char *newp = (char *)realloc((void *)dest, actual_size);
  return newp ? newp : dest;
}

byte *sjis_to_utf8(byte *src, int src_size)
{
  if (!src_size) src_size = strlen((char *)src);

  size_t dest_size = src_size * 2;
  byte *dest = (byte *)malloc(dest_size + 1);

  return (byte *)_iconv((const char *)src, src_size, "Shift_JIS", (char *)dest, dest_size, "UTF-8");
}

byte *utf8_to_sjis(byte *src, int src_size)
{
  if (!src_size) src_size = strlen((char *)src);

  size_t dest_size = src_size * 2;
  byte *dest = (byte *)malloc(dest_size + 1);

  return (byte *)_iconv((const char *)src, src_size, "UTF-8", (char *)dest, dest_size, "Shift_JIS");
}
