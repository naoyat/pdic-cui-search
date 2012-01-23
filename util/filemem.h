// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_FILEMEM_H_
#define UTIL_FILEMEM_H_

#include "util/types.h"

// メモリイメージをそのまま保存
int   savemem(const char* path, byte* data, int data_size, int mode = 0600);
void* loadmem(const char* path);
bool  unloadmem(void* ptr);

int   mem_fd(void* ptr);
int   mem_size(void* ptr);

#endif  // UTIL_FILEMEM_H_
