#include <gtest/gtest.h>

#include "filemem.h"
#include "dump.h"

TEST(filemem, loadmem) {
  EXPECT_EQ( NULL, loadmem(NULL) );
  EXPECT_EQ( NULL, loadmem("") );
  EXPECT_EQ( NULL, loadmem("filename that does not exist") );

  unsigned char *buf = loadmem("filemem_gtest.dat");
  EXPECT_TRUE(buf != NULL);
  EXPECT_EQ(53, mem_size(buf));
  EXPECT_STREQ("ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz", (char *)buf);
  dump(buf, mem_size(buf));
  EXPECT_TRUE( unloadmem(buf) );

  EXPECT_FALSE( unloadmem(buf) );
  EXPECT_FALSE( unloadmem(NULL) );
}

TEST(filemem, unloadmem) {
  EXPECT_FALSE( unloadmem(NULL) );
  EXPECT_FALSE( unloadmem((unsigned char *)"Memory that is not from mmap") );

  unsigned char *buf = loadmem("filemem_gtest.dat");
  ASSERT_TRUE( buf != NULL );
  EXPECT_TRUE( unloadmem(buf) );
  EXPECT_FALSE( unloadmem(buf) ); // cannot unload same mmap
}

TEST(filemem, savemem) {
  unsigned char *sampledata = (unsigned char *)"This is a sample data for savemem.";
  int data_size = strlen((char *)sampledata);
  const char *path = "filemem_gtest.savemem";
  ASSERT_EQ(data_size, savemem(path, sampledata, data_size));

  unsigned char *buf = loadmem(path);
  ASSERT_TRUE( buf != NULL );
  EXPECT_EQ( data_size, mem_size(buf) );
  
  ASSERT_TRUE( unloadmem(buf) );
}

TEST(filemem, mem_size) {
  unsigned char *buf = loadmem("filemem_gtest.dat");
  ASSERT_TRUE(buf != NULL);
  EXPECT_EQ(53, mem_size(buf));
  ASSERT_TRUE( unloadmem(buf) );
}
