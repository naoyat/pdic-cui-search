#include <gtest/gtest.h>

#include <stdlib.h>
#include <string.h>

#include "search.h"
#include "types.h"
#include "util.h"


#ifdef DEBUG
extern int cmp_count;
#endif

// gtest 1.6だとEXPECT_EQ()で行けるんだけど1.5だとエラーが出るので
#define EXPECT_SEARCH_RESULT_EQ(expected,result) do{\
  EXPECT_EQ( expected.first, result.first ); \
  EXPECT_EQ( expected.second.first, result.second.first ); \
  EXPECT_EQ( expected.second.second, result.second.second ); \
  } while(0)

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
  bsearch_result_t result;

  // *exact match*
  // matches once
  result = search(buf, offsets, word_count, (byte*)"aaron", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(0,0)), result );
#ifdef DEBUG
  EXPECT_EQ( 5, cmp_count ); // 1+1 + 3 + 0+0
#endif

  // matches once
  result = search(buf, offsets, word_count, (byte*)"able", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(1,1)), result );
#ifdef DEBUG
  EXPECT_EQ( 7, cmp_count ); // 1+1 + 2 + 1+2
#endif

  // matches twice
  result = search(buf, offsets, word_count, (byte*)"ant", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(2,3)), result );
#ifdef DEBUG
  EXPECT_EQ( 6, cmp_count ); // 1+1 + 3 + 0+1
#endif

  // matches once
  result = search(buf, offsets, word_count, (byte*)"antenna", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(4,4)), result );
#ifdef DEBUG
  EXPECT_EQ( 9, cmp_count ); // 1+1 + 1 + 3+3
#endif

  // matches once
  result = search(buf, offsets, word_count, (byte*)"apple", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(5,5)), result );
#ifdef DEBUG
  EXPECT_EQ( 6, cmp_count ); // 1+1 + 3 + 0+1
#endif

  // matches once
  result = search(buf, offsets, word_count, (byte*)"aunt", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(6,6)), result );
#ifdef DEBUG
  EXPECT_EQ( 6, cmp_count ); // 1+1 + 4 + 0+1
#endif

  // matches once
  result = search(buf, offsets, word_count, (byte*)"bear", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(7,7)), result );
#ifdef DEBUG
  EXPECT_EQ( 8, cmp_count ); // 1+1 + 2 + 4+0
#endif

  // matches once
  result = search(buf, offsets, word_count, (byte*)"best", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(8,8)), result );
#ifdef DEBUG
  EXPECT_EQ( 5, cmp_count ); // 1+1 + 3 + 0
#endif


  // no match ("apartment" can exist between "ant" and "apple")
  result = search(buf, offsets, word_count, (byte*)"apartment", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(4,5)), result );
#ifdef DEBUG
  EXPECT_EQ( 5, cmp_count ); // 1+1 + 3
#endif

  // no match (before the first one)
  result = search(buf, offsets, word_count, (byte*)"aaa", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );
#ifdef DEBUG
  EXPECT_EQ( 1, cmp_count );
#endif

  // no match (after the last one)
  result = search(buf, offsets, word_count, (byte*)"cat", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(8,9)), result );
#ifdef DEBUG
  EXPECT_EQ( 2, cmp_count );
#endif

  // illegal input
  result = search(buf, offsets, word_count, (byte*)"", true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );
#ifdef DEBUG
  EXPECT_EQ( 0, cmp_count );
#endif

  result = search(buf, offsets, word_count, (byte*)NULL, true);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );
#ifdef DEBUG
  EXPECT_EQ( 0, cmp_count );
#endif


  // *non-exact match*
  result = search(buf, offsets, word_count, (byte*)"apple", false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(5,5)), result );

  // matches 3 times ("ant"x2 + "antenna"x1)
  result = search(buf, offsets, word_count, (byte*)"ant", false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(2,4)), result );

  // matches 7 times ("a*")
  result = search(buf, offsets, word_count, (byte*)"a", false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(true,std::make_pair(0,6)), result );

  // no match ("apartment" can exist between "ant" and "apple")
  result = search(buf, offsets, word_count, (byte*)"apartment", false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(4,5)), result );

  // no match (before the first one)
  result = search(buf, offsets, word_count, (byte*)"@@@", false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );

  // no match (after the last one)
  result = search(buf, offsets, word_count, (byte*)"cat", false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(8,9)), result );

  // illegal input
  result = search(buf, offsets, word_count, (byte*)"", false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );

  result = search(buf, offsets, word_count, (byte*)NULL, false);
  EXPECT_SEARCH_RESULT_EQ( std::make_pair(false,std::make_pair(-1,0)), result );

  free((void *)buf);
}

