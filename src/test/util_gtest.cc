// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <gtest/gtest.h>

#include "./types.h"
#include "./util.h"

TEST(util, bstrcmp) {
  EXPECT_EQ(bstrcmp(BYTE(const_cast<char*>("abc")),
                    BYTE(const_cast<char*>("abc"))),
            0);
  EXPECT_LT(bstrcmp(BYTE(const_cast<char*>("abc")),
                    BYTE(const_cast<char*>("abcd"))),
            0);
  EXPECT_GT(bstrcmp(BYTE(const_cast<char*>("abcd")),
                    BYTE(const_cast<char*>("abc"))),
            0);
  EXPECT_LT(bstrcmp(BYTE(const_cast<char*>("abcd")),
                    BYTE(const_cast<char*>("abce"))),
            0);

  EXPECT_EQ(bstrcmp(BYTE(const_cast<char*>("abc\txyz")),
                    BYTE(const_cast<char*>("abc"))),
            0);
  EXPECT_EQ(bstrcmp(BYTE(const_cast<char*>("abc\txyz")),
                    BYTE(const_cast<char*>("abc\tdef"))),
            0);
  EXPECT_EQ(bstrcmp(BYTE(const_cast<char*>("abc")),
                    BYTE(const_cast<char*>("abc\tdef"))),
            0);
}

TEST(util, bstrncmp) {
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("ab")),
                     BYTE(const_cast<char*>("abcde")),
                     2),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abcde")),
                     BYTE(const_cast<char*>("ab")),
                     2),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc")),
                     BYTE(const_cast<char*>("abc")),
                     3),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc")),
                     BYTE(const_cast<char*>("abcd")),
                     3),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abcd")),
                     BYTE(const_cast<char*>("abc")),
                     3),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abcd")),
                     BYTE(const_cast<char*>("abce")),
                     3),
            0);
  EXPECT_LT(bstrncmp(BYTE(const_cast<char*>("abcd")),
                     BYTE(const_cast<char*>("abce")),
                     4),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc\txyz")),
                     BYTE(const_cast<char*>("abc")),
                     3),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc\txyz")),
                     BYTE(const_cast<char*>("abc\tdef")),
                     3),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc")),
                     BYTE(const_cast<char*>("abc\tdef")),
                     3),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc\txyz")),
                     BYTE(const_cast<char*>("abc")),
                     2),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc\txyz")),
                     BYTE(const_cast<char*>("abc\tdef")),
                     2),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc")),
                     BYTE(const_cast<char*>("abc\tdef")),
                     2),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc\txyz")),
                     BYTE(const_cast<char*>("abc")),
                     4),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc\txyz")),
                     BYTE(const_cast<char*>("abc\tdef")),
                     4),
            0);
  EXPECT_EQ(bstrncmp(BYTE(const_cast<char*>("abc")),
                     BYTE(const_cast<char*>("abc\tdef")),
                     4),
            0);
}
