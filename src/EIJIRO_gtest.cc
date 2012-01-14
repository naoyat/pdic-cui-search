#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "Dict.h"
#include "shell.h"
#include "util.h"
#include "dump.h"

extern std::vector<Dict*> dicts;

TEST(EIJIRO, sarray_loookup)
{
  load_rc();
  do_command((char *)"set direct = no");
  do_command((char *)"set coloring = no");

  int dict_id = do_load("EIJI-132.DIC");
  ASSERT_TRUE( dict_id >= 0 );

  Dict *dict = dicts[dict_id];

  lookup_result_vec result = dict->sarray_lookup((byte *)"whose creativity");
  EXPECT_EQ( 3, result.size() );

  EXPECT_STREQ( "man whose creativity seems boundless", (const char *)result[0].entry_word );
  EXPECT_STREQ( "person whose creativity seems boundless", (const char *)result[1].entry_word );
  EXPECT_STREQ( "woman whose creativity seems boundless", (const char *)result[2].entry_word );

  delete dict;
}

TEST(EIJIRO, EIJI_131)
{
  load_rc();
  do_command((char *)"set direct = no");
  do_command((char *)"set coloring = no");

  int dict_id = do_load("EIJI-131.DIC");
  ASSERT_TRUE( dict_id >= 0 );

  Dict *dict = dicts[dict_id];

  lookup_result_vec result = dict->normal_lookup((byte *)"unhate", true);
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
  do_command((char *)"set direct = no");
  do_command((char *)"set coloring = no");

  int dict_id = do_load("EIJI-132.DIC");
  ASSERT_TRUE( dict_id >= 0 );

  Dict *dict = dicts[dict_id];

  lookup_result_vec result = dict->normal_lookup((byte *)"unhate", true);
  EXPECT_EQ( 1, result.size() );

  EXPECT_STREQ( "unhate", (const char *)result[0].entry_word );
  // 【他動】〜を憎むのをやめる、〜に対する憎しみを解く
  EXPECT_STREQ( "【他動】\xef\xbd\x9eを憎むのをやめる、\xef\xbd\x9eに対する憎しみを解く", (const char *)result[0].jword );

  result = dict->normal_lookup((byte *)"claim without foundation", true);
  EXPECT_EQ( 1, result.size() );

  EXPECT_STREQ( "claim without foundation", (const char *)result[0].entry_word );
  // 《a 〜》根拠のない主張
  EXPECT_STREQ( "《a \xef\xbd\x9e》根拠のない主張", (const char *)result[0].jword );

  delete dict;
}
