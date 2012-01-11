#ifndef FILEMEM_H
#define FILEMEM_H

int savemem(const char *path, unsigned char *data, int data_size, int mode=0600); // メモリイメージをそのまま保存
unsigned char *loadmem(const char *path);
bool unloadmem(unsigned char *ptr);

int mem_fd(unsigned char *ptr);
int mem_size(unsigned char *ptr);

#endif
