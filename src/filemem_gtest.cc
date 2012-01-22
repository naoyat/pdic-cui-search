// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <gtest/gtest.h>

#include "./dump.h"
#include "./filemem.h"
#include "./types.h"
#include "./util.h"

TEST(filemem, loadmem) {
  EXPECT_EQ(NULL, loadmem(NULL));
  EXPECT_EQ(NULL, loadmem(""));
  EXPECT_EQ(NULL, loadmem("filename that does not exist"));

  byte *buf = loadmem("filemem_gtest.dat");
  EXPECT_TRUE(buf != NULL);
  EXPECT_EQ(53, mem_size(buf));
  EXPECT_STREQ("ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz",
               reinterpret_cast<char*>(buf));
  dump(buf, mem_size(buf));
  EXPECT_TRUE(unloadmem(buf));

  EXPECT_FALSE(unloadmem(buf));
  EXPECT_FALSE(unloadmem(NULL));
}

TEST(filemem, unloadmem) {
  EXPECT_FALSE(unloadmem(NULL));
  EXPECT_FALSE(unloadmem(
                   BYTE(const_cast<char*>("Memory that is not from mmap"))));

  byte *buf = loadmem("filemem_gtest.dat");
  ASSERT_TRUE(buf != NULL);
  EXPECT_TRUE(unloadmem(buf));
  EXPECT_FALSE(unloadmem(buf));  // cannot unload same mmap
}

TEST(filemem, savemem) {
  byte *sampledata =
      BYTE(const_cast<char*>("This is a sample data for savemem."));
  int data_size = strlen(reinterpret_cast<char*>(sampledata));
  const char *path = "filemem_gtest.savemem";
  ASSERT_EQ(data_size, savemem(path, sampledata, data_size));

  byte *buf = loadmem(path);
  ASSERT_TRUE(buf != NULL);
  EXPECT_EQ(data_size, mem_size(buf));

  ASSERT_TRUE(unloadmem(buf));
}

TEST(filemem, mem_size) {
  byte *buf = loadmem("filemem_gtest.dat");
  ASSERT_TRUE(buf != NULL);
  EXPECT_EQ(53, mem_size(buf));
  ASSERT_TRUE(unloadmem(buf));
}
