#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "Dict.h"

#include "shell.h"
#include "util.h"

extern std::vector<Dict*> dicts;

TEST(Dict, sarray_loookup)
{
  /*
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
  */
}
