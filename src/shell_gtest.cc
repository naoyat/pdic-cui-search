#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "shell.h"
#include "util.h"
#include "dump.h"

TEST(shell, EIJI_131) {
  load_rc();

  bool is_eijiro131_available = do_use("EIJI-131");
  if (is_eijiro131_available) {
    std::vector<std::pair<std::string,std::string> > result;
    std::string entry_word, jword;

    result = lookup((byte *)"unhate");
    EXPECT_EQ( 0, result.size() );

    result = lookup((byte *)"claim without foundation");
    EXPECT_EQ( 0, result.size() );
  }
}

//char *tilde_jisx0221 = "\xe3\x80\x9c"; // U+301C(WAVE DASH)
//char *tilde_ms932    = "\xef\xbd\x9e"; // U+FF5E(FULLWIDTH TILDE)
TEST(shell, EIJI_132) {
  load_rc();

  bool is_eijiro132_available = do_use("EIJI-132");
  if (is_eijiro132_available) {
    std::vector<std::pair<std::string,std::string> > result;
    std::string entry_word, jword;

    result = lookup((byte *)"unhate");
    EXPECT_EQ( 1, result.size() );
    entry_word = result[0].first;
    jword = result[0].second;
    EXPECT_STREQ( "unhate", entry_word.c_str() );
    // 【他動】〜を憎むのをやめる、〜に対する憎しみを解く
    EXPECT_STREQ( "【他動】\xef\xbd\x9eを憎むのをやめる、\xef\xbd\x9eに対する憎しみを解く", jword.c_str() );

    result = lookup((byte *)"claim without foundation");
    EXPECT_EQ( 1, result.size() );
    entry_word = result[0].first;
    jword = result[0].second;
    EXPECT_STREQ( "claim without foundation", entry_word.c_str() );
    // 《a 〜》根拠のない主張
    EXPECT_STREQ( "《a \xef\xbd\x9e》根拠のない主張", jword.c_str() );
  }
}
