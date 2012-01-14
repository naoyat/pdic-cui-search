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

void load_rc(char *rcpath=NULL);
//
// shell commands
//
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
void do_normal_lookup(char *needle, int needle_len=0);
void do_sarray_lookup(char *needle, int needle_len=0);
void do_regexp_lookup(char *needle, int needle_len=0);


// lookup
lookup_result_vec normal_lookup(byte *needle, int needle_len=0);
lookup_result_vec sarray_lookup(byte *needle, int needle_len=0);
lookup_result_vec regexp_lookup(const RE2& pattern);

#endif
