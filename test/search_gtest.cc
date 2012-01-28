// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <gtest/gtest.h>

#include <stdlib.h>
#include <string.h>

#include "./search.h"
#include "./types.h"
#include "./util.h"

// gtest 1.6だとEXPECT_EQ()で行けるんだけど1.5だとエラーが出るので
#define EXPECT_SEARCH_RESULT_EQ(expected, result) do {\
  EXPECT_EQ(expected.first, result.first); \
  EXPECT_EQ(expected.second.first, result.second.first); \
  EXPECT_EQ(expected.second.second, result.second.second); \
  } while (0)

TEST(search, search) {
  //
  // preparation
  //
  const char *some_words[] = {
    "best", "aaron", "ant", "antenna", "ant", "apple", "able", "aunt", "bear"
  };
  ASSERT_STREQ("best", some_words[0]);

  int word_count = sizeof(some_words) / sizeof(const char*);
  ASSERT_EQ(9, word_count);

  qsort(some_words, word_count, sizeof(const char*), pbstrcmp);
  ASSERT_STREQ("aaron", some_words[0]);
  // [ 0:aaron, 1:able, 2:ant, 3:apple, 4:aunt, 5:bear, 6:best ];

  // char **some_words = const_cast<char**>
  std::pair<byte*, int*> buf_and_offset = concat_strings(
      reinterpret_cast<byte**>(const_cast<char**>(some_words)), word_count, 0);

  byte* buf = buf_and_offset.first;
  ASSERT_TRUE(buf != NULL);

  int* offsets = buf_and_offset.second;
  ASSERT_TRUE(offsets != NULL);

  ASSERT_EQ(0, offsets[0]);  // aaron
  ASSERT_EQ(6, offsets[1]);  // able
  ASSERT_EQ(11, offsets[2]);  // ant
  ASSERT_EQ(15, offsets[3]);  // ant
  ASSERT_EQ(19, offsets[4]);  // antenna
  ASSERT_EQ(27, offsets[5]);  // apple
  ASSERT_EQ(33, offsets[6]);  // aunt
  ASSERT_EQ(38, offsets[7]);  // bear
  ASSERT_EQ(43, offsets[8]);  // best

  //
  // actual test
  //
  bsearch_result_t result;

  // *exact match*
  // matches once
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("aaron")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(0, 0)), result);

  // matches once
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("able")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(1, 1)), result);

  // matches twice
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("ant")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(2, 3)), result);

  // matches once
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("antenna")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(4, 4)), result);

  // matches once
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("apple")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(5, 5)), result);

  // matches once
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("aunt")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(6, 6)), result);

  // matches once
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("bear")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(7, 7)), result);

  // matches once
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("best")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(8, 8)), result);


  // no match ("apartment" can exist between "ant" and "apple")
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("apartment")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(4, 5)), result);

  // no match (before the first one)
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("aaa")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(-1, 0)), result);

  // no match (after the last one)
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("cat")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(8, 9)), result);

  // illegal input
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("")), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(-1, 0)), result);

  result = search(buf, offsets, word_count,
                  BYTE(NULL), true);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(-1, 0)), result);


  // *non-exact match*
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("apple")), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(5, 5)), result);

  // matches 3 times ("ant"x2 + "antenna"x1)
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("ant")), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(2, 4)), result);

  // matches 7 times ("a*")
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("a")), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(true, std::make_pair(0, 6)), result);

  // no match ("apartment" can exist between "ant" and "apple")
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("apartment")), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(4, 5)), result);

  // no match (before the first one)
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("@@@")), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(-1, 0)), result);

  // no match (after the last one)
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("cat")), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(8, 9)), result);

  // illegal input
  result = search(buf, offsets, word_count,
                  BYTE(const_cast<char*>("")), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(-1, 0)), result);

  result = search(buf, offsets, word_count,
                  BYTE(NULL), false);
  EXPECT_SEARCH_RESULT_EQ(std::make_pair(false, std::make_pair(-1, 0)), result);

  free(static_cast<void*>(buf));
}

