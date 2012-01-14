#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "shell.h"
#include "util.h"
#include "dump.h"

TEST(shell, sarray_lookup) {
  load_rc();
  do_command("set direct = off");
  do_command("set coloring = off");

  bool is_available = do_use("EIJI-132");
  if (is_available) {
    lookup_result_vec result;
    std::string entry_word, jword;

    result = sarray_lookup((byte *)"whose creativity");
    EXPECT_EQ( 3, result.size() );
    
    EXPECT_STREQ( "man whose creativity seems boundless", (const char *)result[0].entry_word );
    EXPECT_STREQ( "person whose creativity seems boundless", (const char *)result[1].entry_word );
    EXPECT_STREQ( "woman whose creativity seems boundless", (const char *)result[2].entry_word );
  }
}
