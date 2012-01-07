#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatablock.h"
#include "util.h"
#include "bocu1.h"
#include "util_stl.h"

#define rep(var,n)  for(int var=0;var<(n);var++)
#define traverse(c,i)  for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); i++)
#define all(c)  (c).begin(),(c).end()
#define found(s,e)  ((s).find(e)!=(s).end())

#define DEBUG
//#define VERBOSE

#ifdef DEBUG
#include "cout.h"
#endif

typedef struct {
  FILE *fp;
  PDICIndex *index;
} Dict;

// prototypes
void load_rc();
std::vector<int> resolve_aliases(const std::string& name);
void do_use(std::string name);
void do_lookup(char *needle, int needle_len=0);
void lookup(FILE *fp, PDICIndex *index, unsigned char *needle, bool exact_match);

void dump(unsigned char *entry, unsigned char *jword);
int do_load(const std::string& fname);
void do_alias(const std::string& alias, const std::string& valid_name);
void do_alias(const std::string& alias, const std::vector<std::string>& valid_names);
bool do_command(char *cmdstr);

// globals
std::vector<std::string> loadpaths;
std::vector<Dict> dicts;
std::map<std::string,std::vector<std::string> > aliases; // name -> name
std::map<std::string,int> nametable; // name -> dict_id

std::vector<int> current_dict_ids;
std::string current_dict_name = "";

// main()
int main(int argc, char **argv)
{
  load_rc();
  
  if (argc >= 2) {
    std::string fname = argv[1];
    do_load(fname);
  }

  // REPL
  for (bool looping=true; looping; ) {
    char line[256];
    printf("%s> ", current_dict_name.c_str());
    if (!fgets(line, 256, stdin)) { newline(); break; }
    int linelen = strlen(line); line[--linelen] = 0;
    if (linelen == 0) continue;

    switch (line[0]) {
      case '.': // command mode
        looping = do_command(line+1);
        break;

      default:
        do_lookup(line);
        newline();
        break;
    }
  }

  traverse(dicts, dict) {
    fclose(dict->fp);
    delete dict->index;
  }

  return 0;
}

void load_rc()
{
  char rcpath[128];
  strncpy(rcpath, getenv("HOME"), 120);
  strcat(rcpath, "/.pdicrc");

  FILE *fp = fopen(rcpath, "r");
  if (fp != NULL) {
    char line[256];
    while (fgets(line, 256, fp)) {
      int linelen = strlen(line); line[--linelen] = 0;
      if (linelen == 0) continue;

      char *rem = strchr(line, ';');
      if (rem) { *rem = 0; linelen = (int)(rem - line); }

      while (linelen > 0) {
        if (line[linelen-1] != ' ') break;
        line[--linelen] = 0;
      }
      if (linelen == 0) continue;

      do_command(line);
    }
  } else {
    printf(".pdicrc not found\n");
  }
  fclose(fp);
}

std::vector<int> resolve_aliases(const std::string& name)
{
  std::vector<int> dict_ids;
  
  if (found(nametable,name)) {
    dict_ids.push_back( nametable[name] );
  } else if (found(aliases,name)) {
    std::vector<std::string> names = aliases[name];
    traverse(names, name) {
      std::vector<int> ids = resolve_aliases(*name);
      dict_ids.insert(dict_ids.end(), all(ids));
    }
  } else {
    printf("ERROR: '%s' not found.\n", name.c_str());
  }

  return dict_ids;
}

void do_use(std::string name)
{
  std::vector<int> dict_ids = resolve_aliases(name);

  current_dict_ids.assign(all(dict_ids));
  current_dict_name = name;
}

void do_lookup(char *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen(needle);

  if (current_dict_ids.size() == 0) {
    printf("no dictionary selected\n");
    return;
  }

  traverse(current_dict_ids, current_dict_id) {
    Dict dict = dicts[*current_dict_id];
    bool exact_match = true;
    if (needle[needle_len-1] == '*') {
      exact_match = false;
      needle[needle_len-1] = 0;
    }
    lookup(dict.fp, dict.index, (unsigned char *)needle, exact_match);
  }
}


void lookup(FILE *fp, PDICIndex *index, unsigned char *needle, bool exact_match)
{
  Criteria *criteria = new Criteria(needle, exact_match);

  int from, to, cnt;
  cnt = index->bsearch_in_index((unsigned char *)needle, exact_match, from, to);
  for (int ix=from; ix<=to; ix++) {
    if (ix < 0) continue;
    if (ix >= index->nindex) break;

    PDICDatablock* datablock = new PDICDatablock(fp, index, ix);
    datablock->iterate(&dump, criteria);
    delete datablock;
  }
}

void dump(unsigned char *entry, unsigned char *jword)
{
  printf("%s\n", entry);

  unsigned char *jword_indented = (unsigned char *)indent((char *)"   ", (char *)jword);
  printf("%s\n", jword_indented);
  free((char *)jword_indented);
}

int do_load(const std::string& fname)
{
  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + fname;

    FILE *fp = fopen(path.c_str(), "r");
    if (fp != NULL) {
      dicts.push_back( (Dict){fp, new PDICIndex(fp)} );

      int new_dict_id = dicts.size();
      nametable[fname] = new_dict_id;

      dicts.push_back( (Dict){fp, new PDICIndex(fp)} );
      return new_dict_id;
    }
  }
  return -1;
}

void do_alias(const std::string& alias, const std::string& valid_name)
{
  std::vector<std::string> names(1, valid_name);
  do_alias(alias, names);
}
void do_alias(const std::string& alias, const std::vector<std::string>& valid_names)
{
  aliases[alias] = valid_names;
#ifdef VERBOSE
  std::cout << "-" << alias << " -> " << join(valid_names, ", ") << std::endl;
#endif
}

bool do_command(char *cmdstr)
{
  std::vector<std::string> cmd = split(cmdstr);

  if (cmd[0] == "quit" || cmd[0] == "bye") {
    printf("bye.\n");
    return false;
  }
  else if (cmd[0] == "add") {
    if (cmd.size() == 3 && cmd[1] == "loadpath") {
      loadpaths.push_back(cmd[2]);
    } else {
      printf("ERROR: add command supports only 'add loadpath <path>'.\n");
    }
  }
  else if (cmd[0] == "group") {
    std::string groupname = cmd[1];
    std::vector<std::string> names;
    for (int i=2; i<cmd.size(); ++i) {
      if (cmd[i] == "=") continue;
      else names.push_back(cmd[i]);
    }
    do_alias(groupname, names);
  }
  else if (cmd[0] == "load") {
    std::string fname = cmd[1];
    int dict_id = do_load(fname);

    if (dict_id >= 0) {
#ifdef VERBOSE
      std::cout << "+" << fname << std::endl;
#endif
      for (int i=2; i<cmd.size(); ++i) {
        do_alias(cmd[i], fname);
      }
    } else {
      printf("ERROR: file %s not found in loadpaths\n", cmd[1].c_str());
    }
  }
  else if (cmd[0] == "use") {
    std::string name = cmd[1];
    if (found(aliases, name)) {
      do_use(name);
    } else {
      printf("ERROR: '%s' not found.\n", name.c_str());
    }
  }
  else if (cmd[0] == "dump") {
    if (current_dict_ids.size() == 0) {
      printf("ERROR: no dictionary selected\n");
    } else {
      traverse(current_dict_ids, current_dict_id) {
        PDICIndex *index = dicts[*current_dict_id].index;
        std::string what_to_dump = cmd[1];
        if (what_to_dump == "header") {
          index->header->dump();
        } else if (what_to_dump == "index") {
          index->dump();
        } else {
          printf("ERROR: I don't know how to dump '%s'...\n", what_to_dump.c_str());
        }
      }
    }
  }
  else if (cmd[0] == "lookup") {
    do_lookup(cmdstr + 7);
    newline();
  }
  else {
    printf("ERROR: unknown command, '%s'\n", cmd[0].c_str());
  }

  return true;
}

