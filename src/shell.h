#ifndef PDICCUISEARCH_SHELL_H_
#define PDICCUISEARCH_SHELL_H_

#include <string>
#include <set>
#include <vector>
#include <utility>
#include <re2/re2.h>

#include "types.h"

// shell
void shell_init();
void shell_destroy();

void load_rc(const char *rcpath=NULL);
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

#endif // PDICCUISEARCH_SHELL_H_
