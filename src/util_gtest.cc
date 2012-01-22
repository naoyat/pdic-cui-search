// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <gtest/gtest.h>

#include "types.h"
#include "util.h"


TEST(util, bstrcmp) {
  EXPECT_TRUE( bstrcmp((byte*)"abc", (byte*)"abc") == 0 );
  EXPECT_TRUE( bstrcmp((byte*)"abc", (byte*)"abcd") < 0 );
  EXPECT_TRUE( bstrcmp((byte*)"abcd", (byte*)"abc") > 0 );
  EXPECT_TRUE( bstrcmp((byte*)"abcd", (byte*)"abce") < 0 );

  EXPECT_TRUE( bstrcmp((byte*)"abc\txyz", (byte*)"abc") == 0 );
  EXPECT_TRUE( bstrcmp((byte*)"abc\txyz", (byte*)"abc\tdef") == 0 );
  EXPECT_TRUE( bstrcmp((byte*)"abc", (byte*)"abc\tdef") == 0 );
}

TEST(util, bstrncmp) {
  EXPECT_TRUE( bstrncmp((byte*)"ab", (byte*)"abcde", 2) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abcde", (byte*)"ab", 2) == 0 );

  EXPECT_TRUE( bstrncmp((byte*)"abc", (byte*)"abc", 3) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abc", (byte*)"abcd", 3) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abcd", (byte*)"abc", 3) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abcd", (byte*)"abce", 3) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abcd", (byte*)"abce", 4) < 0 );

  EXPECT_TRUE( bstrncmp((byte*)"abc\txyz", (byte*)"abc", 3) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abc\txyz", (byte*)"abc\tdef", 3) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abc", (byte*)"abc\tdef", 3) == 0 );

  EXPECT_TRUE( bstrncmp((byte*)"abc\txyz", (byte*)"abc", 2) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abc\txyz", (byte*)"abc\tdef", 2) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abc", (byte*)"abc\tdef", 2) == 0 );

  EXPECT_TRUE( bstrncmp((byte*)"abc\txyz", (byte*)"abc", 4) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abc\txyz", (byte*)"abc\tdef", 4) == 0 );
  EXPECT_TRUE( bstrncmp((byte*)"abc", (byte*)"abc\tdef", 4) == 0 );
}
