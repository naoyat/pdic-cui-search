#include "bocu1.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util.h"

int decode_trail[256] = {
   -1,   0,   1,   2,   3,   4,   5,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  -1,  -1,  16,  17,  18,  19,
   -1,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
   35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,
   51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,
   67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,
   83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,
   99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
  115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
  131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
  147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162,
  163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178,
  179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
  195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
  211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
  227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242
};

int encode_trail[243] = {
    1,   2,   3,   4,   5,   6,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
   28,  29,  30,  31,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
   45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
   61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,
   77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,

   93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
  109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
  125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140,
  141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156,
  157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172,

  173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
  189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204,
  205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
  221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236,
  237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
  253, 254, 255
};

unsigned char *encode_bocu1(unichar *src_codepoint, int src_len, int& dest_size)
{
  dest_size = 0;
  unsigned char* dest = (unsigned char *)malloc(4*src_len+1);

  for (int i=0,pc=0x40; i<src_len; ) {
    int code = src_codepoint[i++], diff = code - pc;

    if (code <= 0x20) {
      dest[dest_size++] = code;
    } else if (diff < -14536567) {
      // underflow exception
    } else if (diff < -187660) {
      diff -= -14536567;
      int t3 = diff % 243; diff /= 243;
      int t2 = diff % 243; diff /= 243;
      int t1 = diff % 243; diff /= 243;
      // int t0 = diff; // = 0
      dest[dest_size++] = 0x21;
      dest[dest_size++] = encode_trail[t1];
      dest[dest_size++] = encode_trail[t2];
      dest[dest_size++] = encode_trail[t3];
    } else if (diff < -10513) {
      diff -= -187660;
      int t2 = diff % 243; diff /= 243;
      int t1 = diff % 243; diff /= 243;
      int t0 = diff;
      dest[dest_size++] = 0x22 + t0;
      dest[dest_size++] = encode_trail[t1];
      dest[dest_size++] = encode_trail[t2];
    } else if (diff < -64) {
      diff -= -10513;
      int t1 = diff % 243; diff /= 243;
      int t0 = diff;
      dest[dest_size++] = 0x25 + t0;
      dest[dest_size++] = encode_trail[t1];
    } else if (diff < 64) {
      diff -= -64;
      int t0 = diff;
      dest[dest_size++] = 0x50 + t0;
    } else if (diff < 10513) {
      diff -= 64;
      int t1 = diff % 243; diff /= 243;
      int t0 = diff;
      dest[dest_size++] = 0xd0 + t0;
      dest[dest_size++] = encode_trail[t1];
    } else if (diff < 187660) {
      diff -= 10513;
      int t2 = diff % 243; diff /= 243;
      int t1 = diff % 243; diff /= 243;
      int t0 = diff;
      dest[dest_size++] = 0xfb + t0;
      dest[dest_size++] = encode_trail[t1];
      dest[dest_size++] = encode_trail[t2];
    } else if (diff < 14536567) {
      diff -= 187660;
      int t3 = diff % 243; diff /= 243;
      int t2 = diff % 243; diff /= 243;
      int t1 = diff % 243; diff /= 243;
      // int t0 = diff; // = 0
      dest[dest_size++] = 0xfe;
      dest[dest_size++] = encode_trail[t1];
      dest[dest_size++] = encode_trail[t2];
      dest[dest_size++] = encode_trail[t3];
    } else {
      // overflow exception
    }

    // renew pc
    if (code < 0x20) pc = 0x40;
    else if (code == 0x20) ;
    else if (0x3040 <= code && code <= 0x309f) pc = 0x3070;
    else if (0x4e00 <= code && code <= 0x9fa5) pc = 0x7711;
    else if (0xac00 <= code && code <= 0xd7a3) pc = 0xc1d1;
    else pc = (code & 0xffff80) | 0x40;
  }
  dest[dest_size] = 0;

  unsigned char *newp = (unsigned char *)realloc((void *)dest, dest_size+1);
  return newp ? newp : dest;
}

unichar *decode_bocu1(unsigned char *src_bocu1, int src_size, int& dest_size)
{
  dest_size = 0;
  unichar* dest = (unichar *)malloc(sizeof(unichar)*(src_size+1));
  for (int i=0,pc=0x40,lead=0,tr=0,diff=0; i<src_size; ) {
    lead = (unsigned char)src_bocu1[i++];
    if (lead <= 0x20) {
      // code = lead
    }
    else if (lead == 0x21) { // 21 (L T T T)
      diff = -187660 + 243*243*243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr * 243*243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr * 243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr;
    }
    else if (lead < 0x25) { // 22-24 (L T T)
      diff = -10513 + (lead - 0x25)*243*243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr * 243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr;
    }
    else if (lead < 0x50) { // 25-4f (L T)
      diff = -64 + (lead - 0x50)*243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr;
    }
    else if (lead < 0xd0) { // 50-cf (L)
      diff = lead - 0x90;
    }
    else if (lead < 0xfb) { // d0-fa (L T)
      diff = 64 + (lead - 0xd0)*243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr;
    }
    else if (lead < 0xfe) { // fb-fd (L T T)
      diff = 10513 + (lead - 0xfb)*243*243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr * 243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr;
    }
    else if (lead == 0xfe) { // fe (L T T T)
      diff = 187660;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr * 243*243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr * 243;
      tr = decode_trail[ src_bocu1[i++] ];
      diff += tr;
    }
    else { // ff : reset
      ;
    }

    if (lead < 0x20) {
      unichar code = (unichar)lead;
      dest[dest_size++] = code;
      pc = 0x40;
    }
    else if (lead == 0x20) {
      unichar code = 0x20;
      dest[dest_size++] = code;
    } else if (lead < 0xff) {
      unichar code = (unichar)(pc + diff);
      //if (code < 0) code = 0; // error recovery
      dest[dest_size++] = code;
      if (code < 0x20) pc = 0x40;
      else if (code == 0x20) ;
      else if (0x3040 <= code && code <= 0x309f) pc = 0x3070;
      else if (0x4e00 <= code && code <= 0x9fa5) pc = 0x7711;
      else if (0xac00 <= code && code <= 0xd7a3) pc = 0xc1d1;
      else pc = (code & ~0x7f) | 0x40;
    } else {
      pc = 0x40; // reset
    }
  }
  dest[dest_size] = 0;

  unichar *newp = (unichar *)realloc((void *)dest, sizeof(unichar)*(dest_size+1));
  return newp ? newp : dest;
}

unsigned char *utf8_to_bocu1(unsigned char *src_utf8, int src_size)
{
  if (!src_size) src_size = strlen((char *)src_utf8);
                     
  int codepoints_len;
  unichar* codepoints = decode_utf8(src_utf8, src_size, codepoints_len);

  int dest_size;
  unsigned char* dest_bocu1 = encode_bocu1(codepoints, codepoints_len, dest_size);

  free((void *)codepoints);
  return dest_bocu1;
}

unsigned char *bocu1_to_utf8(unsigned char *src_bocu1, int src_size)
{
  if (!src_size) src_size = strlen((char *)src_bocu1);

  int codepoints_len;
  unichar* codepoints = decode_bocu1(src_bocu1, src_size, codepoints_len);

  int dest_size;
  unsigned char* dest_utf8 = encode_utf8(codepoints, codepoints_len, dest_size);

  free((void *)codepoints);
  return dest_utf8;
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



void bocu1_check(unsigned char *bocu1_encoded_data, int size)
{
  if (!size) size = strlen((char *)bocu1_encoded_data);

  printf("["); inline_dump(bocu1_encoded_data, size); printf("]");

  int codepoints1_len;
  unichar *codepoints1 = decode_bocu1(bocu1_encoded_data, size, codepoints1_len);
  printf("\n => {"); inline_dump16(codepoints1, codepoints1_len); printf("} => \n");

  int utf8_len;
  unsigned char *utf8str = encode_utf8(codepoints1, codepoints1_len, utf8_len);
  printf(" => \"%s\" [", utf8str); inline_dump(utf8str, utf8_len); printf("]");

  int codepoints2_len;
  unichar *codepoints2 = decode_utf8(utf8str, utf8_len, codepoints2_len);
  printf(" => {"); inline_dump16(codepoints2, codepoints2_len); printf("}");

  int bocu1_len;
  unsigned char *bocu1str = encode_bocu1(codepoints2, codepoints2_len, bocu1_len);
  printf("["); inline_dump(bocu1str, bocu1_len); printf("]");

  if (codepoints2_len == codepoints1_len && memcmp((char *)codepoints1, (char *)codepoints2, sizeof(unichar)*codepoints1_len) == 0)
    printf(" => ok");
  else
    printf(" => NG");

  if (bocu1_len == size && memcmp((char *)bocu1str, (char *)bocu1_encoded_data, size) == 0)
    printf(" => ok.\n");
  else
    printf(" => NG.\n");

  free((void *)utf8str);
  free((void *)codepoints1);
  free((void *)codepoints2);
  free((void *)bocu1str);
}
