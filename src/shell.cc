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

#include "Dict.h"
#include "PDICDatafield.h"
#include "PDICHeader.h"
#include "PDICIndex.h"
#include "filemem.h"
#include "timeutil.h"
#include "util.h"
#include "util_stl.h"

#include <re2/re2.h>
#include <re2/stringpiece.h>

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

extern bool direct_dump_mode;
extern bool color_mode;
extern bool more_newline;
extern bool verbose_mode;
extern bool whole_mode;
extern int render_count_limit;
//extern int match_count, render_count;

void shell_init()
{
  load_rc();
}
void shell_destroy()
{
  free_all_cloned_buffers();

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
    std::cout << "// .pdicrc が見つかりません。" << std::endl;
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
    std::cout << "// [ERROR] " << name << " が見つかりません。" << std::endl;
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

void do_normal_lookup(char *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen(needle);

  if (current_dict_ids.size() == 0) {
    std::cout << "// 辞書が選択されていません。" << std::endl;
    return;
  }

  reset_match_count();

  RE2 pattern((const char*)needle);
  lookup_result_vec result = normal_lookup((byte *)needle, needle_len);
  lap_match_count();

  if (!direct_dump_mode) {
    traverse(result, it) render_ej(*it, pattern);
  }
  say_match_count();
}

void do_sarray_lookup(char *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen(needle);
  if (needle[needle_len-1] == '*') {
    needle[needle_len-1] = 0;
  }

  if (current_dict_ids.size() == 0) {
    std::cout << "// 辞書が選択されていません。" << std::endl;
    return;
  }

  reset_match_count();

  RE2 pattern((const char *)needle);
  lookup_result_vec result = sarray_lookup((byte*)needle);
  lap_match_count();

  if (!direct_dump_mode) {
    traverse(result, it) render_ej(*it, pattern);
  }
  say_match_count();
}

void do_regexp_lookup(char *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen(needle);
  RE2 pattern(re2::StringPiece(needle, needle_len));

  if (current_dict_ids.size() == 0) {
    std::cout << "// 辞書が選択されていません。" << std::endl;
    return;
  }

  reset_match_count();

  lookup_result_vec result = regexp_lookup(pattern);
  lap_match_count();

  if (!direct_dump_mode) {
    traverse(result, it) render_ej(*it, pattern);
  }
  say_match_count();
}


lookup_result_vec normal_lookup(byte *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen((char *)needle);

  bool exact_match = true;
  if (needle[needle_len-1] == '*') {
    needle[needle_len-1] = 0;
    exact_match = false;
  }

  lookup_result_vec result_total, result;

  traverse(current_dict_ids, current_dict_id) {
    result = dicts[*current_dict_id]->normal_lookup((byte *)needle, exact_match);
    result_total.insert(result_total.end(), all(result));
  }
  std::sort(all(result_total));

  return result_total;
}

lookup_result_vec sarray_lookup(byte *needle, int needle_len)
{
  lookup_result_vec result_total, result;

  traverse(current_dict_ids, current_dict_id) {
    result = dicts[*current_dict_id]->sarray_lookup(needle);
    result_total.insert(result_total.end(), all(result));
  }
  std::sort(all(result_total));

  return result_total;
}

lookup_result_vec regexp_lookup(const RE2& pattern)
{
  lookup_result_vec result_total, result;

  traverse(current_dict_ids, current_dict_id) {
    result = dicts[*current_dict_id]->regexp_lookup(pattern);
    result_total.insert(result_total.end(), all(result));
  }
  std::sort(all(result_total));

  return result_total;
}

int do_load(const std::string& filename)
{
  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + filename;

    byte *filemem = loadmem(path.c_str());
    if (filemem) {
      Dict *new_dict = new Dict(filename, filemem);
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
    std::cout << "しばらくお待ちください..." << std::endl;
    return false;
  }
  else if (cmd[0] == "verbose") {
    std::cout << "verbose mode." << std::endl;
    verbose_mode = true;
  }
  else if (cmd[0] == "quiet") {
    std::cout << "quiet mode." << std::endl;
    verbose_mode = false;
  }
  else if (cmd[0] == "direct") {
    std::cout << "direct dump mode." << std::endl;
    direct_dump_mode = true;
  }
  else if (cmd[0] == "indirect") {
    std::cout << "indirect dump mode." << std::endl;
    direct_dump_mode = false;
  }
  else if (cmd[0] == "whole") {
    std::cout << "whole mode." << std::endl;
    whole_mode = true;
  }
  else if (cmd[0] == "entry") {
    std::cout << "entry mode." << std::endl;
    whole_mode = false;
  }
  else if (cmd[0] == "color") {
    std::cout << "color mode." << std::endl;
    color_mode = true;
  }
  else if (cmd[0] == "plain") {
    std::cout << "plain mode." << std::endl;
    color_mode = false;
  }
  else if (cmd[0] == "newline") {
    if (cmd.size() == 2) {
      if (cmd[1] == "on") {
        std::cout << "newline = on" << std::endl;
        more_newline = true;
      } else if (cmd[1] == "off") {
        std::cout << "newline = off" << std::endl;
        more_newline = false;
      } else {
        std::cout << "[command] newline {on|off} <path>" << std::endl;
      }
    } else {
      std::cout << "[command] newline {on|off} <path>" << std::endl;
    }
  }
  else if (cmd[0] == "add") {
    if (cmd.size() == 3 && cmd[1] == "loadpath") {
      loadpaths.push_back(cmd[2]);
    } else {
      std::cout << "[command] add loadpath <path>" << std::endl;
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
      std::cout << "[command] group <groupname> = <name_1> <name_2> ... <name_n>" << std::endl;
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
        std::cout << "// [ERROR] ファイル " << cmd[1] << " が読み込みパスに見つかりません。" << std::endl;
      }
    } else {
      std::cout << "[command] load <filename>" << std::endl;
    }
  }
  else if (cmd[0] == "use") {
    if (cmd.size() >= 2) {
      std::string name = cmd[1];
      if (found(aliases, name)) {
        do_use(name);
      } else {
        std::cout << "// [ERROR] '" << name << "' が見つかりません。" << std::endl;
      }
    } else {
      std::cout << "[command] use <name>" << std::endl;
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
  else if (cmd.size() == 2 && cmd[0] == "make" && cmd[1] == "toc") {
    traverse(current_dict_ids, current_dict_id) {
      Dict *dict = dicts[*current_dict_id];
      dict->make_toc();
      //dict->load_sarray_index();
    }
  }
  else if (cmd[0] == "dump") {
    if (cmd.size() == 1) {
      std::cout << "[command] dump {header|index|words|datablock <id>|all}" << std::endl;
    } else if (current_dict_ids.size() == 0) {
      std::cout << "// 辞書が選択されていません。" << std::endl;
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
          index->iterate_datablock(ix, &dump_entry, NULL);
        } else if (what_to_dump == "words") {
          index->iterate_all_datablocks(&dump_entry, NULL);
        } else if (what_to_dump == "all") {
          index->iterate_all_datablocks(&dump_ej, NULL);
        } else {
          std::cout << "// [ERROR] I don't know how to dump '" << what_to_dump << "'..." << std::endl;
        }
      }
    }
  }
  else if (cmd[0] == "lookup") {
    do_normal_lookup(cmdstr + 7);
  }
  else if (cmd[0] == "sarray") {
    do_sarray_lookup(cmdstr + 7);
  }
  else if (cmd[0] == "regexp") {
    do_regexp_lookup(cmdstr + 7);
  }
  else {
    printf("ERROR: unknown command, '%s'\n", cmd[0].c_str());
  }

  free_all_cloned_buffers();

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
