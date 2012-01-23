// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "pdic/Dict.h"
#include "util/dump.h"
#include "util/Shell.h"
#include "util/util.h"

TEST(EIJIRO, sarray_loookup) {
  Shell* shell = new Shell();
  shell->load_rc(const_cast<char*>("./gtest.pdicrc"));

  int dict_id = shell->do_load("EIJI-132.DIC");
  ASSERT_GE(dict_id, 0);

  Dict *dict = shell->dicts[dict_id];

  lookup_result_vec result =
      dict->sarray_lookup(BYTE(const_cast<char*>("whose creativity")));
  EXPECT_EQ(3, result.size());

  EXPECT_STREQ("man whose creativity seems boundless",
               reinterpret_cast<char*>(result[0][F_ENTRY]));
  EXPECT_STREQ("person whose creativity seems boundless",
               reinterpret_cast<char*>(result[1][F_ENTRY]));
  EXPECT_STREQ("woman whose creativity seems boundless",
               reinterpret_cast<char*>(result[2][F_ENTRY]));

  delete dict;
  delete shell;
}

TEST(EIJIRO, EIJI_131) {
  Shell* shell = new Shell();
  shell->load_rc(const_cast<char*>("./gtest.pdicrc"));

  int dict_id = shell->do_load("EIJI-131.DIC");
  ASSERT_GE(dict_id, 0);

  Dict *dict = shell->dicts[dict_id];

  lookup_result_vec result =
      dict->normal_lookup(BYTE(const_cast<char*>("unhate")), true);
  EXPECT_EQ(0, result.size());

  result = dict->normal_lookup(
      BYTE(const_cast<char*>("claim without foundation")), true);
  EXPECT_EQ(0, result.size());

  delete dict;
  delete shell;
}

// char *tilde_jisx0221 = "\xe3\x80\x9c"; // U+301C(WAVE DASH)
// char *tilde_ms932    = "\xef\xbd\x9e"; // U+FF5E(FULLWIDTH TILDE)
TEST(Dict, EIJI_132) {
  Shell* shell = new Shell();
  shell->load_rc("./gtest.pdicrc");

  int dict_id = shell->do_load("EIJI-132.DIC");
  ASSERT_GE(dict_id, 0);

  Dict *dict = shell->dicts[dict_id];

  lookup_result_vec result = dict->normal_lookup(
      BYTE(const_cast<char*>("unhate")), true);
  EXPECT_EQ(1, result.size());

  EXPECT_STREQ("unhate", (const char *)result[0][F_ENTRY]);
  // 【他動】〜を憎むのをやめる、〜に対する憎しみを解く
  EXPECT_STREQ(
      "【他動】\xef\xbd\x9eを憎むのをやめる、\xef\xbd\x9eに対する憎しみを解く",
      (const char *)result[0][F_JWORD]);

  result = dict->normal_lookup(
      BYTE(const_cast<char*>("claim without foundation")), true);
  EXPECT_EQ(1, result.size());

  EXPECT_STREQ("claim without foundation",
               (const char *)result[0][F_ENTRY]);
  // 《a 〜》根拠のない主張
  EXPECT_STREQ("《a \xef\xbd\x9e》根拠のない主張",
               (const char *)result[0][F_JWORD]);

  delete dict;
  delete shell;
}
