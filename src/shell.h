#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <utility>
#include <re2/re2.h>

#include "Dict.h"
#include "types.h"

class PDICIndex;
class PDICDatafield;

// shell
void shell_init();
void shell_destroy();

void load_rc();

/// shell commands
bool do_command(char *cmdstr);
// - add loadpath
// - load
int do_load(const std::string& filename);
void do_alias(const std::string& alias, const std::string& valid_name);
void do_alias(const std::string& alias, const std::vector<std::string>& valid_names);
// - group
// * list
// * aliases
// - use
bool do_use(std::string name);
std::vector<int> resolve_aliases(const std::string& name);
// - lookup
void do_lookup(char *needle, int needle_len=0);
void do_regexp_lookup(char *needle, int needle_len=0);
// - dump
void dump_ej(PDICDatafield *datafield); // CALLBACK
void dump_word(PDICDatafield *datafield); // CALLBACK
void dump_to_vector(PDICDatafield *datafield); // CALLBACK
void count_word(PDICDatafield *datafield); // CALLBACK
// - dev
int calculate_space_for_index(PDICIndex *index);


// lookup
lookup_result_vec lookup(byte *needle, int needle_len=0);
lookup_result_vec regexp_lookup(const RE2& pattern);

#endif
