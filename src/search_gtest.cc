#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>

#include "search.h"
#include "util.h"
#include "types.h"

int cmp_count;

TEST(search, concat_strings) {
  EXPECT_TRUE(true);
}

TEST(search, search) {
  //
  // preparation
  //
  const char *some_words[] = { "best","aaron","ant","antenna","ant","apple","able","aunt","bear" };
  ASSERT_STREQ( "best", some_words[0]);

  int word_count = sizeof(some_words) / sizeof(const char*);
  ASSERT_EQ(9, word_count);

  qsort(some_words, word_count, sizeof(const char*), pbstrcmp);
  ASSERT_STREQ( "aaron", some_words[0]);
  // [ 0:aaron, 1:able, 2:ant, 3:apple, 4:aunt, 5:bear, 6:best ];

  std::pair<byte*,int*> buf_and_offset = concat_strings((byte**)some_words, word_count, 0);
  byte*buf = buf_and_offset.first;
  ASSERT_TRUE(buf != NULL);
  int *offsets = buf_and_offset.second;
  ASSERT_TRUE(offsets != NULL);

  ASSERT_EQ(  0, offsets[0] ); // aaron
  ASSERT_EQ(  6, offsets[1] ); // able
  ASSERT_EQ( 11, offsets[2] ); // ant
  ASSERT_EQ( 15, offsets[3] ); // ant
  ASSERT_EQ( 19, offsets[4] ); // antenna
  ASSERT_EQ( 27, offsets[5] ); // apple
  ASSERT_EQ( 33, offsets[6] ); // aunt
  ASSERT_EQ( 38, offsets[7] ); // bear
  ASSERT_EQ( 43, offsets[8] ); // best

  //
  // actual test
  //
  search_result_t result;

  // *exact match*
  // matches once
  result = search(buf, offsets, word_count, (byte*)"aaron", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(0,0)), result );
  EXPECT_EQ( 5, cmp_count ); // 1+1 + 3 + 0+0

  // matches once
  result = search(buf, offsets, word_count, (byte*)"able", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(1,1)), result );
  EXPECT_EQ( 7, cmp_count ); // 1+1 + 2 + 1+2

  // matches twice
  result = search(buf, offsets, word_count, (byte*)"ant", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(2,3)), result );
  EXPECT_EQ( 6, cmp_count ); // 1+1 + 3 + 0+1

  // matches once
  result = search(buf, offsets, word_count, (byte*)"antenna", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(4,4)), result );
  EXPECT_EQ( 9, cmp_count ); // 1+1 + 1 + 3+3

  // matches once
  result = search(buf, offsets, word_count, (byte*)"apple", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(5,5)), result );
  EXPECT_EQ( 6, cmp_count ); // 1+1 + 3 + 0+1

  // matches once
  result = search(buf, offsets, word_count, (byte*)"aunt", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(6,6)), result );
  EXPECT_EQ( 6, cmp_count ); // 1+1 + 4 + 0+1

  // matches once
  result = search(buf, offsets, word_count, (byte*)"bear", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(7,7)), result );
  EXPECT_EQ( 8, cmp_count ); // 1+1 + 2 + 4+0

  // matches once
  result = search(buf, offsets, word_count, (byte*)"best", true);
  EXPECT_EQ( std::make_pair(true,std::make_pair(8,8)), result );
  EXPECT_EQ( 5, cmp_count ); // 1+1 + 3 + 0


  // no match ("apartment" can exist between "ant" and "apple")
  result = search(buf, offsets, word_count, (byte*)"apartment", true);
  EXPECT_EQ( std::make_pair(false,std::make_pair(4,5)), result );
  EXPECT_EQ( 5, cmp_count ); // 1+1 + 3

  // no match (before the first one)
  result = search(buf, offsets, word_count, (byte*)"aaa", true);
  EXPECT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );
  EXPECT_EQ( 1, cmp_count );

  // no match (after the last one)
  result = search(buf, offsets, word_count, (byte*)"cat", true);
  EXPECT_EQ( std::make_pair(false,std::make_pair(8,9)), result );
  EXPECT_EQ( 2, cmp_count );

  // illegal input
  result = search(buf, offsets, word_count, (byte*)"", true);
  EXPECT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );
  EXPECT_EQ( 0, cmp_count );

  result = search(buf, offsets, word_count, (byte*)NULL, true);
  EXPECT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );
  EXPECT_EQ( 0, cmp_count );


  // *non-exact match*
  result = search(buf, offsets, word_count, (byte*)"apple", false);
  EXPECT_EQ( std::make_pair(true,std::make_pair(5,5)), result );

  // matches 3 times ("ant"x2 + "antenna"x1)
  result = search(buf, offsets, word_count, (byte*)"ant", false);
  EXPECT_EQ( std::make_pair(true,std::make_pair(2,4)), result );

  // matches 7 times ("a*")
  result = search(buf, offsets, word_count, (byte*)"a", false);
  EXPECT_EQ( std::make_pair(true,std::make_pair(0,6)), result );

  // no match ("apartment" can exist between "ant" and "apple")
  result = search(buf, offsets, word_count, (byte*)"apartment", false);
  EXPECT_EQ( std::make_pair(false,std::make_pair(4,5)), result );

  // no match (before the first one)
  result = search(buf, offsets, word_count, (byte*)"@@@", false);
  EXPECT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );

  // no match (after the last one)
  result = search(buf, offsets, word_count, (byte*)"cat", false);
  EXPECT_EQ( std::make_pair(false,std::make_pair(8,9)), result );

  // illegal input
  result = search(buf, offsets, word_count, (byte*)"", false);
  EXPECT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );

  result = search(buf, offsets, word_count, (byte*)NULL, false);
  EXPECT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );

  free((void *)buf);
}
