#ifndef PDICCUISEARCH_SEARCH_H_
#define PDICCUISEARCH_SEARCH_H_

#include <utility>
#include "types.h"

typedef std::pair<bool,std::pair<int,int> > bsearch_result_t;

std::pair<byte*,int*> concat_strings(byte *strings[], int string_count, int end_marker=0);
std::pair<int*,int> make_full_suffix_array(byte *buf, int buf_size);
std::pair<int*,int> make_light_suffix_array(byte *buf, int buf_size);

bsearch_result_t search(byte *buf, int *offsets, int offsets_len, byte *needle, bool exact_match);

#endif // PDICCUISEARCH_SEARCH_H_
