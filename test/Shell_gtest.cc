// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <utility>

#include "pdic/Dict.h"
#include "pdic/lookup.h"
#include "util/dump.h"
#include "util/Shell.h"
#include "util/util.h"

TEST(shell, sarray_lookup) {
  Shell shell("./gtest.pdicrc");

  bool is_available = shell.do_use("EIJI-132");
  if (is_available) {
    lookup_result_vec result;

    result = _sarray_lookup(
        BYTE(const_cast<char*>("whose creativity")));
    EXPECT_EQ(3, result.size());
    EXPECT_STREQ("man whose creativity seems boundless",
                 reinterpret_cast<char*>(result[0][F_ENTRY]));
    EXPECT_STREQ("person whose creativity seems boundless",
                 reinterpret_cast<char*>(result[1][F_ENTRY]));
    EXPECT_STREQ("woman whose creativity seems boundless",
                 reinterpret_cast<char*>(result[2][F_ENTRY]));
  }
}
