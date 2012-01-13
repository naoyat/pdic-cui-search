#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "Dict.h"

#include "shell.h"
#include "util.h"
#include "dump.h"

extern std::vector<Dict*> dicts;

TEST(Dict, sarray_loookup)
{
  load_rc();

  int dict_id = do_load("EIJI-132.DIC");
  ASSERT_TRUE( dict_id >= 0 );

  Dict *dict = dicts[dict_id];

  lookup_result_vec result;
  std::string entry_word, jword;

  result = dict->sarray_lookup((byte *)"whose creativity");
  EXPECT_EQ( 3, result.size() );

  EXPECT_STREQ( "man whose creativity seems boundless", result[0].first.c_str() );
  EXPECT_STREQ( "person whose creativity seems boundless", result[1].first.c_str() );
  EXPECT_STREQ( "woman whose creativity seems boundless", result[2].first.c_str() );

  delete dict;
}

TEST(Dict, EIJI_131)
{
  load_rc();

  int dict_id = do_load("EIJI-131.DIC");
  ASSERT_TRUE( dict_id >= 0 );

  Dict *dict = dicts[dict_id];

  lookup_result_vec result;
  std::string entry_word, jword;

  result = dict->normal_lookup((byte *)"unhate", true);
  EXPECT_EQ( 0, result.size() );

  result = dict->normal_lookup((byte *)"claim without foundation", true);
  EXPECT_EQ( 0, result.size() );

  delete dict;
}

//char *tilde_jisx0221 = "\xe3\x80\x9c"; // U+301C(WAVE DASH)
//char *tilde_ms932    = "\xef\xbd\x9e"; // U+FF5E(FULLWIDTH TILDE)
TEST(Dict, EIJI_132)
{
  load_rc();

  int dict_id = do_load("EIJI-132.DIC");
  ASSERT_TRUE( dict_id >= 0 );

  Dict *dict = dicts[dict_id];

  lookup_result_vec result;
  std::string entry_word, jword;

  result = dict->normal_lookup((byte *)"unhate", true);
  EXPECT_EQ( 1, result.size() );
  entry_word = result[0].first;
  jword = result[0].second;
  EXPECT_STREQ( "unhate", entry_word.c_str() );
  // 【他動】〜を憎むのをやめる、〜に対する憎しみを解く
  EXPECT_STREQ( "【他動】\xef\xbd\x9eを憎むのをやめる、\xef\xbd\x9eに対する憎しみを解く", jword.c_str() );

  result = dict->normal_lookup((byte *)"claim without foundation", true);
  EXPECT_EQ( 1, result.size() );
  entry_word = result[0].first;
  jword = result[0].second;
  EXPECT_STREQ( "claim without foundation", entry_word.c_str() );
  // 《a 〜》根拠のない主張
  EXPECT_STREQ( "《a \xef\xbd\x9e》根拠のない主張", jword.c_str() );

  delete dict;
}
