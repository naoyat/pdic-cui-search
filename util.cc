#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "util.h"

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
