// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SRC_FILEMEM_H_
#define SRC_FILEMEM_H_

#include "./types.h"

// メモリイメージをそのまま保存
int   savemem(const char* path, byte* data, int data_size, int mode = 0600);
byte* loadmem(const char* path);
bool  unloadmem(byte* ptr);

int   mem_fd(byte* ptr);
int   mem_size(byte* ptr);

#endif  // SRC_FILEMEM_H_
