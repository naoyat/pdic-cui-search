#ifndef BSEARCH_H
#define BSEARCH_H

#include <utility>

int bsearch_in_sorted_wordlist(unsigned char *buf, int *offset_list, int list_len, unsigned char *needle);
std::pair<int,int> bsearch2_in_sorted_wordlist(unsigned char *buf, int *offset_list, int list_len, unsigned char *needle, bool exact_match);

#endif
