#ifndef PDICCUISEARCH_DUMP_H_
#define PDICCUISEARCH_DUMP_H_

#include <cstdio>
#include "types.h"

char *inline_dump_str(byte *data, int size=0);
char *inline_dump16_str(unichar *data, int size);

void dump(byte *data, int size=0);
void inline_dump(byte *data, int size=0);
void inline_dump16(unichar *data, int size);
void inline_dump16_in_utf8(unichar *data, int size);

void bocu1_dump(byte *bocu1_encoded_data, int size=0);
void bocu1_dump_in_utf8(byte *bocu1_encoded_data, int size=0);

#endif // PDICCUISEARCH_DUMP_H_
