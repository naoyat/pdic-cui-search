#include "search.h"

#include <vector>
#include <queue>
#include <utility>
#include <cstring>

#include "util.h"
#include "util_stl.h"
#include "dump.h"

#ifdef TEST
extern int cmp_count;
#endif

std::pair<byte*,int *> concat_strings(byte *strings[], int string_count, int end_marker)
{
  int offset, *offsets = (int *)malloc(sizeof(int) * (string_count + 1));
  if (!offsets) return std::make_pair((byte *)NULL,(int *)NULL);

  offsets[0] = offset = 0;
  for (int i=0; i<string_count; ++i) {
    offset += strlen((char *)strings[i]) + 1;
    offsets[i+1] = offset;
  }
  if (end_marker) offset++;

  byte* buf = (byte *)malloc(offset), *p = buf;
  if (!buf) { free((void *)offsets); return std::make_pair((byte *)NULL,(int *)NULL); }

  for (int i=0; i<string_count; ++i) {
    int len = offsets[i+1] - offsets[i] - 1;
    memcpy(p, strings[i], len); p += len;
    *(p++) = end_marker;
  }
  if (end_marker) *p = 0;

  return std::make_pair(buf,offsets);
}


byte *buf_; // make_suffix_array() を並列に複数本実行することができない
int bstrcmp_(const void *s1, const void *s2)
{
  return bstrcmp(buf_ + *(int *)s1, buf_ + *(int *)s2);
}

std::pair<int*,int> make_light_suffix_array(byte *buf, int buf_size)
{
  int size_at_most = buf_size;
  int *offsets = (int *)malloc(sizeof(int)*size_at_most), *s = offsets;

  int start_cnt = 0;
  byte last_b = 0;
  for (int i=0; i<buf_size; ++i) {
    byte b = buf[i];
    if (!b) continue;
    if (0x80 <= b && b <= 0xbf) continue;
    if (last_b < 'A') *(s++) = i;
    last_b = b;
  }

  int actual_size = (int)(s - offsets);
  buf_ = buf;
  qsort(offsets, actual_size, sizeof(int), bstrcmp_);
  
  int *newp = (int *)realloc((void *)offsets, sizeof(int)*actual_size);
  if (newp) offsets = newp;

  return std::make_pair(offsets, actual_size);
}


// [NEG-begin, ..., NEG-end, 0-begin, ..., 0-end, POS-begin, ..., POS-end]
// returns {found?, {match-begin = NEG-end+1, match-end = POS_END-1}}
// PDICのindexなど、配列に入っている要素と要素の間に見えない要素がある場合、NEG-endを見るべき
search_result_t search(byte *buf, int *offsets, int offsets_len, byte *needle, bool exact_match)
{
#ifdef TEST
  cmp_count = 0;
#endif

  if (!needle || !needle[0]) return std::make_pair(false, std::make_pair(-1, 0));

  int needle_len = strlen((char *)needle);

  int cmp = exact_match ? bstrcmp(buf+0, needle) : bstrncmp(buf+0, needle, needle_len);
#ifdef TEST
  ++cmp_count;
#endif
  if (cmp > 0) return std::make_pair(false, std::make_pair(-1, 0));

  cmp = exact_match ? bstrcmp(buf+offsets[offsets_len-1], needle) : bstrncmp(buf+offsets[offsets_len-1], needle, needle_len);
#ifdef TEST
  ++cmp_count;
#endif
  if (cmp < 0) return std::make_pair(false, std::make_pair(offsets_len-1, offsets_len));

  int neg_max = -1, pos_min = offsets_len;

  int lo=0, hi=offsets_len;
  while (lo <= hi) {
    int mid = (lo + hi) / 2;

    cmp = exact_match ? bstrcmp(buf+offsets[mid], needle) : bstrncmp(buf+offsets[mid], needle, needle_len);
#ifdef VERBOSE
    /*
    printf("  %d (%d %d %d) %d", exact_match, lo,mid,hi, cmp);
    printf(" ?("); bocu1_dump_in_utf8(buf+offsets[mid]); printf(") ["); bocu1_dump(buf+offsets[mid]); printf("] ");
    printf(" %d(", needle_len); bocu1_dump_in_utf8(needle); printf(") ["); bocu1_dump(needle); printf("]\n");
    */
#endif
    //printf("  %d | %d | %d ", neg_max, mid, pos_min);
    //bocu1_dump_in_utf8(buf+offsets[mid]); newline();
#ifdef TEST
  ++cmp_count;
#endif
    if (cmp == 0) {
      //printf("  %d | =%d | %d\n", neg_max, mid, pos_min);
      // looking for neg-max
      lo = neg_max, hi = mid - 1;
      while (lo < hi) {
        mid = (lo + hi + 1) / 2;
        //printf(" (%d %d %d)\n", lo,mid,hi);
        cmp = exact_match ? bstrcmp(buf+offsets[mid], needle) : bstrncmp(buf+offsets[mid], needle, needle_len);
#ifdef TEST
  ++cmp_count;
#endif
        if (cmp == 0) {
          hi = mid - 1;
        } else {
          lo = mid;
        }
      }
      //printf(";; (%d %d)\n", lo,hi);
      neg_max = hi;

      // looking for pos-min
      lo = mid + 1, hi = pos_min;
      while (lo < hi) {
        mid = (lo + hi) / 2;
        cmp = exact_match ? bstrcmp(buf+offsets[mid], needle) : bstrncmp(buf+offsets[mid], needle, needle_len);
#ifdef TEST
  ++cmp_count;
#endif
        if (cmp == 0) {
          lo = mid + 1;
        } else {
          hi = mid;
        }
      }
      //printf(";; (%d %d)\n", lo,hi);
      pos_min = lo;

      return std::make_pair(true, std::make_pair(neg_max+1, pos_min-1));
    }
    else if (cmp < 0) {
      //printf("  %d | <%d | %d\n", neg_max, mid, pos_min);
      if (mid > neg_max) neg_max = mid;
      lo = mid + 1;
      continue;
    }
    else if (cmp > 0) {
      //printf("  %d | >%d | %d\n", neg_max, mid, pos_min);
      if (mid < pos_min) pos_min = mid;
      hi = mid - 1;
      continue;
    }
  }
  return std::make_pair(false, std::make_pair(neg_max, pos_min));
}
