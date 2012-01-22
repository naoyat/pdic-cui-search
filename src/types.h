#ifndef PDICCUISEARCH_TYPES_H_
#define PDICCUISEARCH_TYPES_H_

#include <vector>

class PDICDatafield;

typedef unsigned char byte;
typedef unsigned short unichar;
typedef unsigned int uint;

typedef byte *byteptr;
typedef byteptr *lookup_result;
typedef lookup_result *lookup_result_ptr;
typedef std::vector<lookup_result> lookup_result_vec;
typedef lookup_result_vec (lookup_proc)(byte *needle, int needle_len);

typedef void (action_proc)(PDICDatafield*);

#endif // PDICCUISEARCH_TYPES_H_
