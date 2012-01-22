// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "dump.h"
#include "lookup.h"
#include "shell.h"
#include "util.h"


TEST(shell, sarray_lookup) {
  load_rc("./gtest.pdicrc");
  bool is_available = do_use("EIJI-132");

  if (is_available) {
    lookup_result_vec result;

    result = _sarray_lookup((byte *)"whose creativity");
    EXPECT_EQ( 3, result.size() );
    EXPECT_STREQ( "man whose creativity seems boundless", (const char *)result[0][F_ENTRY] );
    EXPECT_STREQ( "person whose creativity seems boundless", (const char *)result[1][F_ENTRY] );
    EXPECT_STREQ( "woman whose creativity seems boundless", (const char *)result[2][F_ENTRY] );
  }
}
