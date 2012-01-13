#include "search.h"

#include <vector>
#include <queue>
#include <utility>
#include <cstring>

#include "util.h"
#include "util_stl.h"
#include "types.h"
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

std::pair<int*,int> make_full_suffix_array(byte *buf, int buf_size)
{
  int size_at_most = buf_size;
  int *offsets = (int *)malloc(sizeof(int)*size_at_most), *s = offsets;

  for (int i=0; i<buf_size; ++i) *(s++) = i;

  int actual_size = (int)(s - offsets);
  buf_ = buf;
  qsort(offsets, actual_size, sizeof(int), bstrcmp_);

  int *newp = (int *)realloc((void *)offsets, sizeof(int)*actual_size);
  if (newp) offsets = newp;

  return std::make_pair(offsets, actual_size);
}

std::pair<int*,int> make_light_suffix_array(byte *buf, int buf_size)
{
  int size_at_most = buf_size;
  int *offsets = (int *)malloc(sizeof(int)*size_at_most), *s = offsets;

  bool last_alpha = false;
  for (int i=0; i<buf_size; ) {
    byte b = *(buf + i);
    if (b <= 0x20) {
      ++i;
      last_alpha = false;
    } else if (b < 0x80) { // 00-7F: [UTF-8] 1 letter
      if (isalpha((int)b)) {
        if (!last_alpha) *(s++) = i;
        last_alpha = true;
      } else {
        last_alpha = false;
      }
      ++i;
    } else if (b < 0xC0) { // 80-BF: [UTF-8] 先頭以外
      last_alpha = false;
      ++i;
    } else if (b < 0xE0) { // C0-DF: [UTF-8] 2 letters
      *(s++) = i; i += 2;
      last_alpha = false;
    } else if (b < 0xF0) { // E0-EF: [UTF-8] 3 letters
      *(s++) = i; i += 3;
      last_alpha = false;
    } else if (b < 0xF8) { // F0-F7: [UTF-8] 4 letters
      *(s++) = i; i += 4;
      last_alpha = false;
    } else if (b < 0xFC) { // F8-FB: [UTF-8] 5 letters
      *(s++) = i; i += 5;
      last_alpha = false;
    } else if (b < 0xFE) { // FC-FD: [UTF-8] 6 letters
      *(s++) = i; i += 6;
      last_alpha = false;
    } else { // FE,FF
      ++i;
      last_alpha = false;
    }
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
bsearch_result_t search(byte *buf, int *offsets, int offsets_len, byte *needle, bool exact_match)
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
