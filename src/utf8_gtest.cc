#include <gtest/gtest.h>

#include "utf8.h"
#include "types.h"

TEST(utf8, surrogate) {
  int upper, lower;

  EXPECT_EQ(false, surrogate(0x0, &upper, &lower));
  EXPECT_EQ(-1, upper);
  EXPECT_EQ(-1, lower);

  EXPECT_EQ(false, surrogate(0xffff, &upper, &lower));
  EXPECT_EQ(-1, upper);
  EXPECT_EQ(-1, lower);

  EXPECT_EQ(true, surrogate(0x10000, &upper, &lower));
  EXPECT_EQ(0xD800, upper);
  EXPECT_EQ(0xDC00, lower);
  EXPECT_EQ(true, surrogate(0x103ff, &upper, &lower));
  EXPECT_EQ(0xD800, upper);
  EXPECT_EQ(0xDFFF, lower);
  EXPECT_EQ(true, surrogate(0x10400, &upper, &lower));
  EXPECT_EQ(0xD801, upper);
  EXPECT_EQ(0xDC00, lower);

  EXPECT_EQ(true, surrogate(0x10ffff, &upper, &lower));
  EXPECT_EQ(0xDBFF, upper);
  EXPECT_EQ(0xDFFF, lower);
  
  EXPECT_EQ(false, surrogate(0x110000, &upper, &lower));
  EXPECT_EQ(-1, upper);
  EXPECT_EQ(-1, lower);
}

TEST(utf8, unsurrogate) {
  EXPECT_EQ(0x010000, unsurrogate(0xD800, 0xDC00));
  EXPECT_EQ(0x0103FF, unsurrogate(0xD800, 0xDFFF));
  EXPECT_EQ(0x010400, unsurrogate(0xD801, 0xDC00));
  EXPECT_EQ(0x10FFFF, unsurrogate(0xDBFF, 0xDFFF));

  // illegal
  EXPECT_EQ(-1, unsurrogate(0xD7FF, 0xDC00));
  EXPECT_EQ(-1, unsurrogate(0xDC00, 0xDC00));
  EXPECT_EQ(-1, unsurrogate(0xD800, 0xDBFF));
  EXPECT_EQ(-1, unsurrogate(0xD800, 0xE000));
}

TEST(utf8, decode_utf8) {
  int dest_len;
  unichar *dest;
  int expected_codepoint, upper, lower;

  // U+0000
  dest = decode_utf8((byte *)"\x00", 1, dest_len);
  expected_codepoint = 0;
  EXPECT_EQ(1, dest_len);
  EXPECT_EQ(expected_codepoint, dest[0]);
  free(dest);

  // U+007F
  dest = decode_utf8((byte *)"\x7f", 1, dest_len);
  expected_codepoint = 0x7f;
  EXPECT_EQ(1, dest_len);
  EXPECT_EQ(expected_codepoint, dest[0]);
  free(dest);

  // U+0080
  dest = decode_utf8((byte *)"\xc2\x80", 2, dest_len);
  expected_codepoint = 0x0080;
  EXPECT_EQ(1, dest_len);
  EXPECT_EQ(expected_codepoint, dest[0]);
  free(dest);

  // U+07FF
  dest = decode_utf8((byte *)"\xdf\xbf", 2, dest_len);
  expected_codepoint = 0x07ff;
  EXPECT_EQ(1, dest_len);
  EXPECT_EQ(expected_codepoint, dest[0]);
  free(dest);

  // U+0800
  dest = decode_utf8((byte *)"\xe0\xa0\x80", 3, dest_len);
  expected_codepoint = 0x0800;
  EXPECT_EQ(1, dest_len);
  EXPECT_EQ(expected_codepoint, dest[0]);
  free(dest);

  // U+0FFFF
  dest = decode_utf8((byte *)"\xef\xbf\xbf", 3, dest_len);
  expected_codepoint = 0xffff;
  EXPECT_EQ(1, dest_len);
  EXPECT_EQ(expected_codepoint, dest[0]);
  free(dest);

  // U+010000
  dest = decode_utf8((byte *)"\xf0\x90\x80\x80", 4, dest_len);
  expected_codepoint = 0x10000;
  surrogate(expected_codepoint, &upper, &lower);
  EXPECT_EQ(2, dest_len);
  EXPECT_EQ(upper, dest[0]);
  EXPECT_EQ(lower, dest[1]);
  free(dest);

  // U+10FFFF
  dest = decode_utf8((byte *)"\xf4\x8f\xbf\xbf", 4, dest_len);
  expected_codepoint = 0x10ffff;
  surrogate(expected_codepoint, &upper, &lower);
  EXPECT_EQ(2, dest_len);
  EXPECT_EQ(upper, dest[0]);
  EXPECT_EQ(lower, dest[1]);
  free(dest);

  // U+110000
  dest = decode_utf8((byte *)"\xf4\x90\x80\x80", 4, dest_len);
  EXPECT_EQ(0, dest_len);
  free(dest);

  // U+1FFFFF
  dest = decode_utf8((byte *)"\xf7\xbf\xbf\xbf", 4, dest_len);
  EXPECT_EQ(0, dest_len);
  free(dest);

  // U+200000
  dest = decode_utf8((byte *)"\xf8\x88\x80\xa0\x80", 5, dest_len);
  EXPECT_EQ(0, dest_len);
  free(dest);

  // U+03FFFFFF
  dest = decode_utf8((byte *)"\xfb\xbf\xbf\xbf\xbf", 5, dest_len);
  EXPECT_EQ(0, dest_len);
  free(dest);

  // U+04000000
  dest = decode_utf8((byte *)"\xfc\x84\x80\x80\xa0\x80", 6, dest_len);
  EXPECT_EQ(0, dest_len);
  free(dest);

  // U+7FFFFFFF
  dest = decode_utf8((byte *)"\xfd\xbf\xbf\xbf\xbf\xbf", 6, dest_len);
  EXPECT_EQ(0, dest_len);
  free(dest);
}

TEST(utf8, encode_utf8) {
  unichar src[2];
  int dest_size;

  EXPECT_STREQ("", (const char *)encode_utf8(src, 0, dest_size));
  EXPECT_EQ(0, dest_size);

  // U+00
  src[0] = 0;
  EXPECT_STREQ("", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(0, dest_size); // NULL終端して扱われるため

  // U+01
  src[0] = 1;
  EXPECT_STREQ("\x1", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(1, dest_size);

  // U+07F
  src[0] = 0x7f;
  EXPECT_STREQ("\x7f", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(1, dest_size);

  // U+080
  src[0] = 0x80;
  EXPECT_STREQ("\xc2\x80", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(2, dest_size);

  // U+07FF
  src[0] = 0x7ff;
  EXPECT_STREQ("\xdf\xbf", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(2, dest_size);

  // U+0800
  src[0] = 0x800;
  EXPECT_STREQ("\xe0\xa0\x80", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(3, dest_size);

  // U+0FFFF
  src[0] = 0xFFFF;
  EXPECT_STREQ("\xef\xbf\xbf", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(3, dest_size);

  // U+10000
  src[0] = 0xD800; src[1] = 0xDC00;
  EXPECT_STREQ("\xf0\x90\x80\x80", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(4, dest_size);

  // U+10FFFF
  src[0] = 0xDBFF; src[1] = 0xDFFF;
  EXPECT_STREQ("\xf4\x8f\xbf\xbf", (const char *)encode_utf8(src, 1, dest_size));
  EXPECT_EQ(4, dest_size);

  // U+0040 U+0040
  src[0] = 0x40; src[1] = 0x40;
  EXPECT_STREQ("\x40\x40", (const char *)encode_utf8(src, 2, dest_size));
  EXPECT_EQ(2, dest_size);
}
