#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "shell.h"
#include "util.h"
#include "dump.h"

TEST(shell, sarray_lookup) {
  load_rc();

  bool is_available = do_use("EIJI-132");
  if (is_available) {
    std::vector<std::pair<std::string,std::string> > result;
    std::string entry_word, jword;

    result = sarray_lookup((byte *)"whose creativity");
    EXPECT_EQ( 3, result.size() );
    
    EXPECT_STREQ( "man whose creativity seems boundless", result[0].first.c_str() );
    EXPECT_STREQ( "person whose creativity seems boundless", result[1].first.c_str() );
    EXPECT_STREQ( "woman whose creativity seems boundless", result[2].first.c_str() );
  }
}
