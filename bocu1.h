#ifndef __BOCU1_H
#define __BOCU1_H

#include "util.h"

// codepoints <=> bocu1
unsigned char *encode_bocu1(unichar *src_codepoint, int src_len, int& dest_size);
unichar *decode_bocu1(unsigned char *src_bocu1, int src_size, int& dest_size);

// utf8 <=> bocu1 (via codepoints), returns NULL-ending string
unsigned char *utf8_to_bocu1(unsigned char *src_utf8, int src_size=0);//, int& dest_size);
unsigned char *bocu1_to_utf8(unsigned char *src_bocu1, int src_size=0);//, int& dest_size);

void bocu1_check(unsigned char *bocu1_encoded_data, int size=0);

#endif
