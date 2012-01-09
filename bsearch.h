#ifndef __BSEARCH_H
#define __BSEARCH_H

int bsearch_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle);
int bsearch2_in_sorted_wordlist(unsigned char **list, int list_len, unsigned char *needle, bool exact_match, int& lo, int& hi);

#endif
