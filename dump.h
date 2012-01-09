#ifndef DUMP_H
#define DUMP_H

#include <cstdio>
#include "util.h"

void dump(unsigned char *data, int size=0);
void inline_dump(unsigned char *data, int size=0);
void inline_dump16(unsigned short *data, int size);
void inline_dump16_in_utf8(unsigned short *data, int size);

void bocu1_dump(unsigned char *bocu1_encoded_data, int size=0);
void bocu1_dump_in_utf8(unsigned char *bocu1_encoded_data, int size=0);

inline void newline() { putchar('\n'); }

#endif
