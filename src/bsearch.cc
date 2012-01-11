#include "bsearch.h"

#include <cstring>
#include "util.h"

// 交点１箇所を仮定したもの
int bsearch_in_sorted_wordlist(unsigned char *buf, int *offset_list, int list_len, unsigned char *needle)
{
  //  int needle_len = strlen((char *)needle);
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
    int cmp = bstrcmp(buf + offset_list[mid], needle);
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

std::pair<int,int> bsearch2_in_sorted_wordlist(unsigned char *buf, int *offset_list, int list_len, unsigned char *needle, bool exact_match)
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
      cmp = bstrcmp(buf + offset_list[mid], needle);
      //putchar('L'); // 比較した回数だけ表示
    } else {
      cmp = bstrncmp(buf + offset_list[mid], needle, needle_len);
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
      cmp = bstrcmp(buf + offset_list[mid], needle);
      //putchar('H'); // 比較した回数だけ表示
    } else {
      cmp = bstrncmp(buf + offset_list[mid], needle, needle_len);
      //putchar('h'); // 比較した回数だけ表示
    }

    if (cmp <= 0) lo = mid+1; // (mid+1, hi)
    else {
      hi = mid; // (lo, mid)
    }
  }
  hi_min = lo;

  //putchar('\n');

  // list[lo+1]==needleだとしても、list[lo]のブロックにneedleにひっかかる物があるかもしれないので、
  // lo++とかはしないでおく
  return std::make_pair(lo_max, hi_min-1);
}
