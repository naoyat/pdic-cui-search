// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./filemem.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <map>
#include <string>
#include <utility>

// #include "./stlutil.h"

std::map<void*, std::pair<int, int> > mmap_info;

// メモリイメージをそのまま保存
int savemem(const char* path, byte* data, int data_size, int mode) {
  int fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
  if (fd == -1) return -1;

  ssize_t saved_size = write(fd, static_cast<void*>(data), data_size);
  close(fd);

  return static_cast<int>(saved_size);
}

void* loadmem(const char* path) {
  int fd = open(path, O_RDONLY);
  if (fd == -1) return NULL;

  off_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  int size = file_size;

  void *ptr = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    close(fd);
    return NULL;
  }

  mmap_info[ptr] = std::make_pair(fd, size);
  return ptr;
}

bool unloadmem(void* ptr) {
  if (mmap_info.find(ptr) == mmap_info.end())  // if not found
    return false;

  std::pair<int, int> info = mmap_info[ptr];
  int fd = info.first, size = info.second;

  munmap(ptr, size);
  close(fd);

  mmap_info.erase(ptr);
  return true;
}

int mem_fd(void* ptr) {
  if (mmap_info.find(ptr) == mmap_info.end())  // if not found
    return -1;

  std::pair<int, int> info = mmap_info[ptr];
  int fd = info.first;
  return fd;
}

int mem_size(void *ptr) {
  if (mmap_info.find(ptr) == mmap_info.end())  // if not found
    return -1;

  std::pair<int, int> info = mmap_info[ptr];
  int size = info.second;
  return size;
}
