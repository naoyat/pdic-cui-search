#include "filemem.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <map>
#include <string>
#include <utility>

#include "util_stl.h"


std::map<void*,std::pair<int,int> > mmap_info;

// メモリイメージをそのまま保存
int savemem(const char *path, byte *data, int data_size, int mode)
{
  int fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
  if (fd == -1) return -1;

  ssize_t saved_size = write(fd, (const void *)data, (size_t)data_size);
  close(fd);

  return (int)saved_size;
}

byte *loadmem(const char *path)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1) return NULL;

  off_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  //int psize = getpagesize();
  int size = file_size;
  
  void *ptr = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) { close(fd); return NULL; }

  mmap_info[ptr] = std::make_pair(fd, size);
  return (byte *)ptr;
}

bool unloadmem(byte *ptr)
{
  if (!found(mmap_info, (void *)ptr)) return false;

  std::pair<int,int> info = mmap_info[(void *)ptr];
  int fd = info.first, size = info.second;

  munmap(ptr, size);
  close(fd);

  mmap_info.erase((void *)ptr);
  return true;
}

int mem_fd(byte *ptr)
{
  if (!found(mmap_info, (void *)ptr)) return -1;

  std::pair<int,int> info = mmap_info[(void *)ptr];
  int fd = info.first/*, size = info.second*/;
  return fd;
}
int mem_size(byte *ptr)
{
  if (!found(mmap_info, (void *)ptr)) return -1;

  std::pair<int,int> info = mmap_info[(void *)ptr];
  int /* fd = info.first, */ size = info.second;
  return size;
}
