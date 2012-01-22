#ifndef PDICCUISEARCH_FILEMEM_H_
#define PDICCUISEARCH_FILEMEM_H_

#include "types.h"

int savemem(const char *path, byte *data, int data_size, int mode=0600); // メモリイメージをそのまま保存
byte *loadmem(const char *path);
bool unloadmem(byte *ptr);

int mem_fd(byte *ptr);
int mem_size(byte *ptr);

#endif // PDICCUISEARCH_FILEMEM_H_
