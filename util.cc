#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util.h"
//#include "bocu1.h"

#include <iconv.h>

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

unsigned char *encode_utf8(unichar *src_codepoint, int src_size, int& dest_size)
{
  dest_size = 0;
  for (int i=0; i<src_size; i++) {
    int codepoint = src_codepoint[i];
    if (codepoint == 0) break;
    else if (codepoint <= 0x7f) dest_size += 1;
    else if (codepoint <= 0x07ff) dest_size += 2;
    else /* if (src_codepoint[i] <= 0xffff) */ {
      if (0xd800 <= codepoint && codepoint <= 0xdbff) {
        //int upper = codepoint - 0xd800;
        //int lower = src_codepoint[++i] - 0xdc00;
        dest_size += 4;
      } else {
        dest_size += 3;
      }
    }
  }

  unsigned char *dest = (unsigned char *)malloc(dest_size + 1);
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
unichar *decode_utf8(unsigned char *src_utf8, int src_size, int& dest_size)
{
  dest_size = 0;

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
      dest[dest_size++] = codepoint;
    } else if (codepoint < 0x110000) {
      int upper, lower;
      surrogate(codepoint, &upper, &lower);
      //printf("{%x -> %04x:%04x}", codepoint, upper, lower);
      dest[dest_size++] = upper;
      dest[dest_size++] = lower;
    } else {
      // simply ignore
    }
  }

  dest[dest_size] = 0;

  unichar *newp = (unichar *)realloc((void *)dest, sizeof(unichar)*(dest_size+1));
  return newp ? newp : dest;
}

// 交点１箇所を仮定したもの
int bsearch_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle)
{
  // list が辞書順にソート済みであり、
  // strcmp(list[i], needle) == 0 となる i（これをi*としよう）が高々１つしかない
  // ことを想定している

  // i*が0個 →needleより小さい最後のiを返す
  // i*が1個 →そのi*を返す
  // i*が複数個（これはこの関数では想定していない）→i*のどれかを返す

  int lo = 0, hi = list_len;
  // 実際には比較は行われないが、任意の合法な needle に対し strcmp(list[hi], needle) が常に正であるような値が list[hi] に門番として入っているのを想像して下さい
  while (lo+2 <= hi) {
    // needle must be found in [lo, hi)
    int mid = (lo + hi) / 2; // cannot be ==lo
    int cmp = strcmp((const char *)list[mid], (const char *)needle);
    //putchar('.'); // 比較した回数だけ表示
    //printf("[%d, %d, %d) ", lo, mid, hi);
    //printf(" comparing ["); bocu1_dump_in_utf8(list[mid]); printf("]");
    //printf(" and ["); bocu1_dump_in_utf8(needle); printf("]");
    //printf(" --> %d\n", cmp);
    if (cmp == 0) return mid;
    else {
      if (cmp < 0) lo = mid; // (mid, hi)
      else         hi = mid; // (lo, mid)
    }
  }
  return lo;
}

int bsearch2_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle, bool exact_match, int& oLo, int& oHi)
{
  //strcmp(list[i], needle) == 0 となる i（これをi*としよう）が２つ以上あるかもしれないリストの探索
  int lo_max = -1, hi_min = list_len;
  int needle_len = strlen((char *)needle);
  
  int lo = 0, hi = list_len, mid = 0, cmp = 0;
  // 実際には比較は行われないが、任意の合法な needle に対し strcmp(list[hi], needle) が常に正であるような値が list[hi] に門番として入っているのを想像して下さい
  while (lo < hi) {
    // at last, needle must not be found in [lo, hi)
    mid = (lo + hi + 1) / 2; // cannot be ==lo
    if (mid == list_len) break;

    //printf("[lo=%d, mid=%d, hi=%d] ", lo,mid,hi);
    if (exact_match) {
      cmp = strcmp((const char *)list[mid], (const char *)needle);
      //putchar('L'); // 比較した回数だけ表示
    } else {
      cmp = strncmp((const char *)list[mid], (const char *)needle, needle_len);
      //putchar('l'); // 比較した回数だけ表示
    }

    if (cmp >= 0) hi = mid-1; // [lo, mid-1)
    else {
      lo = mid; // (mid, hi)
    }
  }
  lo_max = lo;

  //
  lo = lo_max+1, hi = list_len, mid = 0;
  while (lo < hi) {
    // needle must not be found in [lo, hi)
    mid = (lo + hi) / 2; // cannot be ==lo
    if (exact_match) {
      cmp = strcmp((const char *)list[mid], (const char *)needle);
      //putchar('H'); // 比較した回数だけ表示
    } else {
      cmp = strncmp((const char *)list[mid], (const char *)needle, needle_len);
      //putchar('h'); // 比較した回数だけ表示
    }

    if (cmp <= 0) lo = mid+1; // (mid+1, hi)
    else {
      hi = mid; // (lo, mid)
    }
  }
  hi_min = lo;

  //putchar('\n');

  oLo = lo_max, oHi = hi_min-1;
  // list[lo+1]==needleだとしても、list[lo]のブロックにneedleにひっかかる物があるかもしれないので、
  // lo++とかはしないでおく

  return oHi - oLo + 1;
}

char *indent(char *spacer, char *src)
{
  int len_before = strlen(src);
  int len_spacer = strlen(spacer);
  // pass1
  int nl = 0;
  for (char *p=src; *p; ++p) {
    if (*p == '\n') nl++;
  }
  int len_after = len_before + (1+nl)*len_spacer;
  char *indented = (char *)malloc(len_after + 1); indented[len_after] = 0;

  strcpy(indented, spacer); char *dest = indented + len_spacer;
  for (char *p=src; *p; ++p) {
    *(dest++) = *p;
    if (*p == '\n') { strcpy(dest, spacer); dest += len_spacer; }
  }
  return indented;
}

unsigned char *cstr(unsigned char *data, int length)
{
  if (!length) length = strlen((char *)data);

  unsigned char *buf = (unsigned char *)malloc(length + 1);
  memcpy(buf, data, length); data[length] = 0;
  return buf;
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

unsigned char *sjis_to_utf8(unsigned char *src, int src_size)
{
  if (!src_size) src_size = strlen((char *)src);

  size_t dest_size = src_size * 2;
  unsigned char *dest = (unsigned char *)malloc(dest_size + 1);

  return (unsigned char *)_iconv((const char *)src, src_size, "Shift_JIS", (char *)dest, dest_size, "UTF-8");
}

unsigned char *utf8_to_sjis(unsigned char *src, int src_size)
{
  if (!src_size) src_size = strlen((char *)src);

  size_t dest_size = src_size * 2;
  unsigned char *dest = (unsigned char *)malloc(dest_size + 1);

  return (unsigned char *)_iconv((const char *)src, src_size, "UTF-8", (char *)dest, dest_size, "Shift_JIS");
}
