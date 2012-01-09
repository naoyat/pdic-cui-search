#ifndef __DUMP_H_
#define __DUMP_H_

#include <cstdio>
#include "util.h"

void dump(unsigned char *data, int size=0);
void inline_dump(unsigned char *data, int size=0);
void inline_dump16(unsigned short *data, int size);
void inline_dump16_in_utf8(unsigned short *data, int size);

inline void newline() { putchar('\n'); }

#endif
