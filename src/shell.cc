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
#include "ansi_color.h"

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

extern bool separator_mode;
extern bool verbose_mode;
extern bool direct_dump_mode;
extern bool full_search_mode;
extern bool ansi_coloring_mode;
extern bool more_newline_mode;
extern int render_count, render_count_limit;

lookup_result_vec current_result_vec;
RE2* current_pattern;
std::pair<std::string,std::string> current_query;
extern std::set<void*> clone_ptrs;
void render_status()
{
  if (current_pattern) {
    std::cout << ANSI_FGCOLOR_GREEN "// 最後に行われた検索: (" << current_query.first << ") \"" << current_query.second << "\"" ANSI_FGCOLOR_DEFAULT << std::endl;
    //std::cout << "// 最後に行われた検索: /" << current_pattern->pattern() << "/" << std::endl;
    std::cout << ANSI_FGCOLOR_GREEN "// 現在保持している結果: " << current_result_vec.size() << "件" ANSI_FGCOLOR_DEFAULT << std::endl;
  }
  //std::cout << "clone_ptrs.size()=" << clone_ptrs.size();
  //std::cout << std::endl;
}

void shell_init()
{
  load_rc();
}
void shell_destroy()
{
  free_all_cloned_buffers();

  traverse(dicts, dict) delete *dict;
}

void load_rc(char *rcpath)
{
  char buf[256];
  if (!rcpath) {
    strncpy(buf, getenv("HOME"), 119);
    strcat(buf, "/.pdicrc");
    rcpath = buf;
  }

  FILE *fp = fopen(rcpath, "r");
  if (fp != NULL) {
    while (fgets(buf, 256, fp)) {
      int linelen = strlen(buf); buf[--linelen] = 0;
      if (linelen == 0) continue;

      char *rem = strchr(buf, ';');
      if (rem) { *rem = 0; linelen = (int)(rem - buf); }

      while (linelen > 0) {
        if (buf[linelen-1] != ' ') break;
        buf[--linelen] = 0;
      }
      if (linelen == 0) continue;

      do_command(buf);
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

void render_current_result()
{
  traverse(current_result_vec, it) render_result(*it, current_pattern);
}
void render_current_result(const std::set<int>& range)
{
  traverse(range, it) {
    lookup_result *result = current_result_vec[*it];
    render_count = 0;
    render_result(result, current_pattern);
  }
  render_count = range.size();
}

void do_normal_lookup(char *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen(needle);

  if (current_dict_ids.size() == 0) {
    std::cout << "// 辞書が選択されていません。" << std::endl;
    return;
  }

  char needle_pattern[4+needle_len+1];
  sprintf(needle_pattern, "(?i)%s", needle);
  if (needle[needle_len-1] == '*') {
    needle_pattern[4+needle_len-1] = 0;
  }

  reset_match_count(); reset_render_count();
  current_pattern = new RE2(needle_pattern);
  lookup_result_vec result_vec = normal_lookup((byte *)needle, needle_len);
  current_result_vec.assign(all(result_vec));
  lap_match_count();
  if (!direct_dump_mode) render_current_result();
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

  reset_match_count(); reset_render_count();
  current_pattern = new RE2((const char *)needle);
  lookup_result_vec result_vec = sarray_lookup((byte*)needle);
  current_result_vec.assign(all(result_vec));
  lap_match_count();
  if (!direct_dump_mode) render_current_result();
  say_match_count();
}

void do_regexp_lookup(char *needle, int needle_len)
{
  if (!needle_len) needle_len = strlen(needle);

  if (current_dict_ids.size() == 0) {
    std::cout << "// 辞書が選択されていません。" << std::endl;
    return;
  }

  reset_match_count(); reset_render_count();
  current_pattern = new RE2(re2::StringPiece(needle, needle_len));
  lookup_result_vec result_vec = regexp_lookup(current_pattern);
  current_result_vec.assign(all(result_vec));
  lap_match_count();
  if (!direct_dump_mode) render_current_result();
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
  current_query = std::make_pair(exact_match ? "exact" : "match-forward", (const char *)needle);

  lookup_result_vec total_result_vec;
  traverse(current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec = dicts[*current_dict_id]->normal_lookup((byte *)needle, exact_match);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec sarray_lookup(byte *needle, int needle_len)
{
  current_query = std::make_pair("suffix-array", (const char *)needle);

  lookup_result_vec total_result_vec;
  traverse(current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec = dicts[*current_dict_id]->sarray_lookup(needle);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
}

lookup_result_vec regexp_lookup(RE2 *current_pattern)
{
  current_query = std::make_pair("regexp", current_pattern->pattern());

  lookup_result_vec total_result_vec;
  traverse(current_dict_ids, current_dict_id) {
    lookup_result_vec result_vec = dicts[*current_dict_id]->regexp_lookup(current_pattern);
    total_result_vec.insert(total_result_vec.end(), all(result_vec));
  }
  std::sort(all(total_result_vec), lookup_result_asc);
  return total_result_vec;
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

  if (cmd[0][0] == 'q' /*|| cmd[0] == "quit"*/ || cmd[0] == "bye") {
    std::cout << "しばらくお待ちください..." << std::endl;
    return false;
  }
  else if (cmd[0] == "status") {
    render_status();
  }
  else if (cmd[0] == "last" || cmd[0] == "again") {
    int last = current_result_vec.size();
    if (last > 0) {
      reset_render_count();
      if (cmd.size() >= 2) {
        std::set<int> lines;
        int s, e, line;
        for (int i=1; i<cmd.size(); ++i) {
          if (RE2::FullMatch(cmd[i],"(\\d+)-(\\d+)", &s, &e))  {
            if (s < 1) s = 1;
            if (last < e) e = last;
            if (s <= e) for (line=s; line<=e; ++line) lines.insert(line-1);
          }
          else if (RE2::FullMatch(cmd[i],"(\\d+)-", &s))  {
            if (s < 1) s = 1;
            if (s <= last) for (line=s; line<=last; ++line) lines.insert(line-1);
          }
          else if (RE2::FullMatch(cmd[i],"-(\\d+)", &e))  {
            if (last < e) e = last;
            if (1 <= e) for (line=1; line<=e; ++line) lines.insert(line-1);
          }
          else if (RE2::FullMatch(cmd[i],"(\\d+)", &line)) {
            if (1 <= line && line <= last) lines.insert(line-1);
          }
        }
#ifdef DEBUG
        std::cout << "lines: " << lines << std::endl;
#endif
        render_current_result(lines);
      } else {
        render_current_result();
      }
      say_render_count();
    }
  }
  else if (cmd[0] == "set") {
    if (cmd.size() >= 3) {
      std::cout << ANSI_FGCOLOR_GREEN << "// ";
      int value_ix = 2; if (cmd[2] == "=") ++value_ix;
      bool onoff = cmd[value_ix] == "on";
      const char *onoff_str = onoff ? "ON" : "OFF";
      if (cmd[1] == "verbose") {
        verbose_mode = onoff;
        std::cout << "verbose mode = " << onoff_str << std::endl;
      }
      else if (cmd[1] == "direct") {
        direct_dump_mode = onoff;
        std::cout << "direct dump mode = " << onoff_str << std::endl;
      }
      else if (cmd[1] == "separator") {
        separator_mode = onoff;
        std::cout << "separator mode = " << onoff_str << std::endl;
      }
      else if (cmd[1] == "full" || cmd[1] == "full_search") {
        full_search_mode = onoff;
        std::cout << "full search mode = " << onoff_str << std::endl;
      }
      else if (cmd[1] == "coloring" || cmd[1] == "ansi_coloring") {
        ansi_coloring_mode = onoff;
        std::cout << "ANSI coloring mode = " << onoff_str << std::endl;
      }
      else if (cmd[1] == "newline" || cmd[1] == "more_newline") {
        more_newline_mode = onoff;
        std::cout << "newline = " << onoff_str << std::endl;
      }
      else if (cmd[1] == "limit" || cmd[1] == "render_limit" || cmd[1] == "render_count_limit") {
        int num = atoi( cmd[value_ix].c_str() );
        render_count_limit = (num > 0) ? num : DEFAULT_RENDER_COUNT_LIMIT;
        std::cout << "render count limit = " << render_count_limit << std::endl;
      }
      std::cout << ANSI_FGCOLOR_DEFAULT;
    } else {
      std::cout << "[command] set {limit} = <number>" << std::endl;
      std::cout << "[command] set {verbose|separator|direct|full|coloring|newline} = {on|off}" << std::endl;
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
#ifdef DEBUG
        std::cout << "DUMP: " << current_dict_name << " " << current_dict_ids << std::endl;
#endif
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
          index->iterate_datablock(ix, &cb_dump_entry, NULL);
        } else if (what_to_dump == "words") {
          index->iterate_all_datablocks(&cb_dump_entry, NULL);
        } else if (what_to_dump == "all") {
          index->iterate_all_datablocks(&cb_dump, NULL);
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
  else if (cmd[0] == "clean") {
    free_all_cloned_buffers();
  }
  else {
    std::cout << "// [ERROR] 未知のコマンド '" << cmd[0] << "' に遭遇..." << std::endl;
  }

  return true;
}
