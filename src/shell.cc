#include "shell.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Criteria.h"
#include "Dict.h"
#include "PDICDatablock.h"
#include "PDICDatafield.h"
#include "PDICHeader.h"
#include "PDICIndex.h"

#include "bocu1.h"
#include "dump.h"
#include "search.h"
#include "timeutil.h"
#include "util.h"
#include "util_stl.h"

#ifdef DEBUG
#include "cout.h"
#endif

//
// globals
//
std::vector<std::string> loadpaths;
std::vector<Dict*> dicts;
std::map<std::string,std::vector<std::string> > aliases; // name -> name
std::map<std::string,int> nametable; // name -> dict_id
std::vector<int> current_dict_ids;
std::string current_dict_name = "";

bool verbose_mode = false;
bool more_newline = false;

void shell_init()
{
  load_rc();
}
void shell_destroy()
{
  traverse(dicts, dict) delete *dict;
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

bool do_use(std::string name)
{
  std::vector<int> dict_ids = resolve_aliases(name);

  if (dict_ids.size() > 0) {
    current_dict_ids.assign(all(dict_ids));
    current_dict_name = name;
    return true;
  } else {
    return false;
  }
}

void render_ej(byte *entry_word, byte *jword)
{
  std::cout << entry_word << std::endl;

  if (jword && jword[0]) {
    byte *jword_indented = (byte *)indent((char *)"   ", (char *)jword);
    std::cout << jword_indented << std::endl;
    free(jword_indented);
  }

  if (more_newline) std::cout << std::endl;
}

void do_lookup(char *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen(needle);

  if (verbose_mode) {
    std::cout << "LOOKUP: " << current_dict_name << " " << current_dict_ids << std::endl;
  }

  std::vector<std::pair<std::string,std::string> > result = lookup((byte *)needle, needle_len);
  traverse(result, it) {
    render_ej((byte *)it->first.c_str(), (byte *)it->second.c_str());
  }
}

lookup_result_vec lookup(byte *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen((char *)needle);

  if (current_dict_ids.size() == 0) {
    printf("no dictionary selected\n");
    return lookup_result_vec();
  }

  lookup_result_vec result_total, result;

  traverse(current_dict_ids, current_dict_id) {
    Dict *dict = dicts[*current_dict_id];
    bool exact_match = true;
    if (needle[needle_len-1] == '*') {
      needle[needle_len-1] = 0;
      exact_match = false;
    }

    if (needle[0] == '*' && needle_len >= 2) {
      // suffix array search
      std::vector<int> matches = dict->search_in_sarray((byte *)needle+1);
      traverse(matches, offset) {
        //printf("- %s\n", dict->entry_buf + *offset);
        byte *entry_word = dict->entry_buf + *offset;
        result_total.push_back( std::make_pair((const char *)entry_word, "") );
      }
      //printf("%d matches.\n", (int)matches.size());
      //continue;
    } else {
      // normal search
      result = dict->normal_lookup((byte *)needle, exact_match);
      result_total.insert(result_total.end(), all(result));
    }
  }
  std::sort(all(result_total));

  return result_total;
}

void dump_ej(PDICDatafield *datafield)
{
  render_ej( datafield->entry_word_utf8(), datafield->jword_utf8() );
}
void dump_word(PDICDatafield *datafield)
{
  puts((char *)datafield->entry_word_utf8());
}

int do_load(const std::string& filename)
{
  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + filename;

    FILE *fp = fopen(path.c_str(), "r");
    if (fp != NULL) {
      Dict *new_dict = new Dict(fp, filename, path);
      int new_dict_id = dicts.size();
      dicts.push_back(new_dict);
      nametable[new_dict->suffix()] = new_dict_id;

      if (verbose_mode) {
        printf("loading %s... => { name: %s, dict_id: %d }\n", path.c_str(), new_dict->suffix(), new_dict_id);
      }
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
}

bool do_command(char *cmdstr)
{
  std::vector<std::string> cmd = split(cmdstr);

  if (cmd[0] == "quit" || cmd[0] == "bye") {
    printf("bye.\n");
    return false;
  }
  else if (cmd[0] == "verbose") {
    printf("verbose mode.\n");
    verbose_mode = true;
  }
  else if (cmd[0] == "quiet") {
    printf("quiet mode.\n");
    verbose_mode = false;
  }
  else if (cmd[0] == "newline") {
    if (cmd.size() == 2) {
      if (cmd[1] == "on") {
        printf("newline = on\n");
        more_newline = true;
      } else if (cmd[1] == "off") {
        printf("newline = off\n");
        more_newline = false;
      } else {
        printf("[command] newline {on|off} <path>\n");
      }
    } else {
      printf("[command] newline {on|off} <path>\n");
    }
  }
  else if (cmd[0] == "add") {
    if (cmd.size() == 3 && cmd[1] == "loadpath") {
      loadpaths.push_back(cmd[2]);
    } else {
      printf("[command] add loadpath <path>\n");
    }
  }
  else if (cmd[0] == "group" && cmd.size() >= 2) {
    if (cmd.size() >= 2) {
      std::string groupname = cmd[1];
      std::vector<std::string> names;
      for (int i=2; i<cmd.size(); ++i) {
        if (cmd[i] == "=") continue;
        else names.push_back(cmd[i]);
      }
      do_alias(groupname, names);
    } else {
      printf("[command] group <groupname> = <name_1> <name_2> ... <name_n>\n");
    }
  }
  else if (cmd[0] == "load") {
    if (cmd.size() >= 2) {
      std::string filename = cmd[1];
      int dict_id = do_load(filename);

      if (dict_id >= 0) {
        char *name = dicts[dict_id]->suffix();
        //std::cout << "+" << name << std::endl;
        for (int i=2; i<cmd.size(); ++i) {
          do_alias(cmd[i], name);
        }
      } else {
        printf("ERROR: file %s not found in loadpaths\n", cmd[1].c_str());
      }
    } else {
      printf("[command] load <filename>\n");
    }
  }
  else if (cmd[0] == "use") {
    if (cmd.size() >= 2) {
      std::string name = cmd[1];
      if (found(aliases, name)) {
        do_use(name);
      } else {
        printf("ERROR: '%s' not found.\n", name.c_str());
      }
    } else {
      printf("[command] use <name>\n");
    }
  }
  else if (cmd[0] == "list") {
    std::set<int> dict_ids(all(current_dict_ids));
    for (int dict_id=0; dict_id<dicts.size(); ++dict_id) {
      printf("%2d%c %s\n", dict_id, (found(dict_ids,dict_id) ? '*' : ':'), dicts[dict_id]->info().c_str());
    }
  }
  else if (cmd[0] == "aliases") {
    traverse(aliases, alias) {
      std::cout << alias->first << ": " << join(alias->second, ", ") << std::endl;
    }
    /*
    traverse(nametable, name) {
      int dict_id = name->second;
      std::cout << name->first << ": " << dict_id << " " << dicts[dict_id]->info() << std::endl;
    }
    */
  }
  else if (cmd[0] == "make") {
    if (cmd.size() == 2 && cmd[1] == "index") {
      traverse(current_dict_ids, current_dict_id) {
        Dict *dict = dicts[*current_dict_id];
        dict->make_sarray_index();
        //dict->load_sarray_index();
      }
    } else {
      printf("[command] make index\n");
    }
  }
  else if (cmd[0] == "dump") {
    if (cmd.size() >= 2) {
      if (current_dict_ids.size() == 0) {
        printf("ERROR: no dictionary selected\n");
      } else {
        if (verbose_mode) {
          std::cout << "DUMP: " << current_dict_name << " " << current_dict_ids << std::endl;
        }
        traverse(current_dict_ids, current_dict_id) {
          PDICIndex *index = dicts[*current_dict_id]->index;
          std::string what_to_dump = cmd[1];
          if (what_to_dump == "header") {
            index->header->dump();
          } else if (what_to_dump == "index") {
            index->dump();
          } else if (what_to_dump == "datablock" && cmd.size() >= 3) {
            int ix = atoi( cmd[2].c_str() );
            index->iterate_datablock(ix, &dump_word, NULL);
          } else if (what_to_dump == "words") {
            index->iterate_all_datablocks(&dump_word, NULL);
          } else if (what_to_dump == "count") {
            printf("[%d]", *current_dict_id);
            calculate_space_for_index(index);
          } else if (what_to_dump == "all") {
            index->iterate_all_datablocks(&dump_ej, NULL);
          } else {
            printf("ERROR: I don't know how to dump '%s'...\n", what_to_dump.c_str());
          }
        }
      }
    } else {
      printf("[command] dump {header|index|words|datablock <id>|all}\n");
    }
  }
  /*
  else if (cmd[0] == "dev") {
    if (cmd.size() >= 2) {
      if (current_dict_ids.size() == 0) {
        printf("ERROR: no dictionary selected\n");
      } else {
        // std::cout << "DUMP: " << current_dict_name << " " << current_dict_ids << std::endl;
        traverse(current_dict_ids, current_dict_id) {
          PDICIndex *index = dicts[*current_dict_id]->index;
          std::string subcmd = cmd[1];
          if (subcmd == "makeindex") {
            printf("[%d]", *current_dict_id);
            make_index(index);
          } else {
            printf("ERROR: illegal command: '%s'...\n", subcmd.c_str());
          }
        }
      }
    } else {
      printf("[command] dev {makeindex}\n");
    }
  }
  */
  else if (cmd[0] == "lookup") {
    do_lookup(cmdstr + 7);
  }
  else {
    printf("ERROR: unknown command, '%s'\n", cmd[0].c_str());
  }

  return true;
}

int _wordcount = 0;
int _wordsize_sum = 0;

int calculate_space_for_index(PDICIndex *index)
{
  time_reset();

  _wordcount = 0;
  _wordsize_sum = 0;
  //_words.clear();

  index->iterate_all_datablocks(&count_word, NULL);

  std::pair<int,int> time = time_usec();

  printf("%d words, %d bytes; %.2f b/w; real:%d process:%d.\n", _wordcount, _wordsize_sum, (double)_wordsize_sum/_wordcount, time.first, time.second);

  return _wordsize_sum;
}

void count_word(PDICDatafield *datafield)
{
  ++_wordcount;
  _wordsize_sum += (datafield->entry_index_size + 1);
}
