// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "pdic/lookup.h"
#include "util/ansi_color.h"
#include "util/dump.h"
#include "pdic/Shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <re2/re2.h>

#include <limits>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "pdic/Dict.h"
#include "pdic/Dict_callbacks.h"
#include "pdic/lookup.h"
#include "pdic/PDICDatafield.h"
#include "pdic/PDICHeader.h"
#include "pdic/PDICIndex.h"
#include "util/ansi_color.h"
#include "util/filemem.h"
#include "util/stlutil.h"
#include "util/timeutil.h"
#include "util/util.h"
#include "util/macdic_xml.h"
#include "util/sqlite3_sql.h"

extern int sqlite3_sql_entry_id_;
extern Dict *_dict;

Shell *g_shell = NULL;

Shell::Shell() {
  g_shell = this;
}

Shell::~Shell() {
  g_shell = NULL;
  free_all_cloned_buffers();
  traverse(dicts, dict) delete *dict;
}

int Shell::run() {
  printf(ANSI_UNDERLINE_ON
         "PDIC CUI Search ver 0.7.2 (c)2012 @naoya_t. All Rights Reserved."
         ANSI_UNDERLINE_OFF "\n");

  // printf("読み込み中...\n");
  g_shell->load_rc();

  // if (argc >= 2) {
  //   std::string filename = argv[1];
  //   g_shell->do_load(filename);
  // }

  // REPL
  for (bool looping = true; looping; ) {
    printf("%s> ", g_shell->current_dict_name.c_str());
    char line[256];
    if (!fgets(line, 256, stdin)) {
      printf("\n");
      break;
    }
    int linelen = strlen(line);
    line[--linelen] = 0;
    if (linelen == 0) continue;

    int head = line[0], tail = line[linelen-1];
    switch (head) {
      case '?':  // reserved (help)
        break;

      case '+':
        lookup(reinterpret_cast<byte*>(line)+1, LOOKUP_FROM_ALL);
        break;

      case '.':  // command mode
        if (linelen > 1) {
          looping = g_shell->do_command(line+1);
        }
        break;

      case '!':  // external shell mode
        if (linelen > 1) {
          system(line+1);
        }
        break;

      case '*':
        if (linelen > 1) {
          lookup(reinterpret_cast<byte*>(line)+1, LOOKUP_SARRAY);
        }
        break;

      case '/':
        if (linelen >= 3 && tail == '/') {
          line[linelen-1] = '\0';
          lookup(reinterpret_cast<byte*>(line)+1, LOOKUP_REGEXP);
          break;
        }
        // else fall thru

      default:
        int flags = g_shell->params.default_lookup_flags;
        if (tail == '*') {
          line[linelen-1] = '\0';
          flags &= ~LOOKUP_MATCH_BACKWARD;
        }
        lookup(reinterpret_cast<byte*>(line), flags);
        break;
    }
  }

  printf("bye!\n");
}

void Shell::load_rc(const char* rcpath) {
  char buf[256];
  if (!rcpath) {
    snprintf(buf, sizeof(buf), "%s/.pdicrc", getenv("HOME"));
    rcpath = buf;
  }

  FILE* fp = fopen(rcpath, "r");
  if (fp != NULL) {
    while (fgets(buf, 256, fp)) {
      int linelen = strlen(buf);
      buf[--linelen] = 0;

      if (linelen == 0) continue;

      char* rem = strchr(buf, ';');
      if (rem) {
        *rem = 0;
        linelen = static_cast<int>(rem - buf);
      }

      while (linelen > 0) {
        if (buf[linelen-1] != ' ') break;
        buf[--linelen] = 0;
      }
      if (linelen == 0) continue;

      do_command(buf);
    }
  } else {
    printf("// .pdicrc が見つかりません。\n");
  }
  fclose(fp);
}

int Shell::do_load(const std::string& filename) {
  for (uint i = 0; i < loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + filename;

    byte* filemem = static_cast<byte*>(loadmem(path.c_str()));
    if (filemem) {
      Dict* new_dict = new Dict(filename, filemem);
      int new_dict_id = dicts.size();
      dicts.push_back(new_dict);
      nametable[new_dict->prefix()] = new_dict_id;

      if (params.verbose_mode) {
        printf("loading %s... => { name: %s, dict_id: %d }\n",
               path.c_str(), new_dict->prefix(), new_dict_id);
      }
      return new_dict_id;
    }
  }
  return -1;
}

bool Shell::do_command(char *cmdstr) {
  std::vector<std::string> cmd = split(cmdstr);

  if (cmd[0][0] == 'q' /*|| cmd[0] == "quit"*/ || cmd[0] == "bye") {
    printf("しばらくお待ちください...\n");
    return false;
  } else if (cmd[0] == "status") {
    render_status();
  } else if (cmd[0] == "last" || cmd[0] == "again") {
    int last = current_result_vec.size();
    if (last > 0) {
      reset_render_count();
      if (cmd.size() >= 2) {
        std::set<int> lines;
        int s, e, line;
        for (uint i = 1; i < cmd.size(); ++i) {
          if (RE2::FullMatch(cmd[i], "(\\d+)-(\\d+)", &s, &e))  {
            if (s < 1) s = 1;
            if (last < e) e = last;
            if (s <= e) {
              for (line = s; line <= e; ++line)
                lines.insert(line - 1);
            }
          } else if (RE2::FullMatch(cmd[i], "(\\d+)-", &s))  {
            if (s < 1) s = 1;
            if (s <= last) {
              for (line = s; line <= last; ++line)
                lines.insert(line - 1);
            }
          } else if (RE2::FullMatch(cmd[i], "-(\\d+)", &e))  {
            if (last < e) e = last;
            if (1 <= e) {
              for (line = 1; line <= e; ++line)
                lines.insert(line - 1);
            }
          } else if (RE2::FullMatch(cmd[i], "(\\d+)", &line)) {
            if (1 <= line && line <= last)
              lines.insert(line - 1);
          }
        }
        render_current_result(lines);
      } else {
        render_current_result();
      }
      say_render_count();
    }
  } else if (cmd[0] == "set") {
    if (cmd.size() >= 3) {
      int value_ix = 2;
      if (cmd[2] == "=") ++value_ix;
      const char *mode_name = NULL;
      char value_str[11] = { 0 };
      if (cmd[value_ix] == "on" || cmd[value_ix] == "off") {
        bool onoff = cmd[value_ix] == "on";
        snprintf(value_str, sizeof(value_str), onoff ? "ON" : "OFF");
        if (cmd[1] == "verbose") {
          params.verbose_mode = onoff;
          mode_name = "verbose mode";
        } else if (cmd[1] == "direct") {
          params.direct_dump_mode = onoff;
          mode_name = "direct dump mode";
        } else if (cmd[1] == "separator") {
          params.separator_mode = onoff;
          mode_name = "separator mode";
        } else if (cmd[1] == "full" || cmd[1] == "full_search") {
          params.full_search_mode = onoff;
          mode_name = "full search mode";
        } else if (cmd[1] == "coloring" || cmd[1] == "ansi_coloring") {
          params.ansi_coloring_mode = onoff;
          mode_name = "ANSI coloring mode";
        } else if (cmd[1] == "newline" || cmd[1] == "more_newline") {
          params.more_newline_mode = onoff;
          mode_name = "newline";
        } else if (cmd[1] == "stop_on_limit") {
          params.stop_on_limit_mode = onoff;
          mode_name = "stop on limit";
        } else {
          mode_name = NULL;
          value_str[0] = '\0';
        }
      } else {
        if (cmd[1] == "limit"
            || cmd[1] == "render_limit"
            || cmd[1] == "render_count_limit") {
          int num = atoi(cmd[value_ix].c_str());
          params.set_render_count_limit(num);
          mode_name = "render count limit";
          snprintf(value_str, sizeof(value_str), "%d",
                   params.render_count_limit);
        } else if (cmd[1] == "lookup") {
          mode_name = "default lookup mode";
          snprintf(value_str, sizeof(value_str), "%s", cmd[value_ix].c_str());

          if (params.set_lookup_mode(cmd[value_ix].c_str()) != 0) {
            printf("[command] "
                   "set lookup = {exact|forward|sarray|regexp|all}\n");
            mode_name = NULL;
          }
        } else {
          mode_name = NULL;
          value_str[0] = '\0';
        }
      }
      if (params.verbose_mode && mode_name != NULL) {
        printf("%s", ANSI_FGCOLOR_GREEN);
        printf("// %s = %s\n", mode_name, value_str);
        printf("%s", ANSI_FGCOLOR_DEFAULT);
      }
    } else {
      printf("[command] set {limit} = <number>\n");
      printf("[command] set "
             "{verbose|separator|direct|full|coloring|newline} = {on|off}\n");
    }
  } else if (cmd[0] == "add") {
    if (cmd.size() == 3 && cmd[1] == "loadpath") {
      loadpaths.push_back(cmd[2]);
    } else {
      printf("[command] add loadpath <path>\n");
    }
  } else if (cmd[0] == "group" && cmd.size() >= 2) {
    if (cmd.size() >= 2) {
      std::string groupname = cmd[1];
      std::vector<std::string> names;
      for (uint i = 2; i < cmd.size(); ++i) {
        if (cmd[i] == "=") continue;
        names.push_back(cmd[i]);
      }
      do_alias(groupname, names);
    } else {
      printf("[command] group <groupname> = <name_1> <name_2> ... <name_n>\n");
    }
  } else if (cmd[0] == "load") {
    if (cmd.size() >= 2) {
      std::string filename = cmd[1];
      int dict_id = do_load(filename);
      
      if (dict_id >= 0) {
        char *name = dicts[dict_id]->prefix();
        // printf("+" << name << std::endl;
        for (uint i = 2; i < cmd.size(); ++i)
          do_alias(cmd[i], name);
      } else {
        printf("// [ERROR] ファイル %s が読み込みパスに見つかりません。\n",
               cmd[1].c_str());
      }
    } else {
      printf("[command] load <filename>\n");
    }
  } else if (cmd[0] == "use") {
    if (cmd.size() >= 2) {
      std::string name = cmd[1];
      if (aliases.find(name) != aliases.end()) {  // if found
        do_use(name);
      } else {
        printf("// [ERROR] '%s' が見つかりません。\n", name.c_str());
      }
    } else {
      printf("[command] use <name>\n");
    }
  } else if (cmd[0] == "list") {
    std::set<int> dict_ids(current_dict_ids.begin(), current_dict_ids.end());
    for (uint dict_id = 0; dict_id < dicts.size(); ++dict_id) {
      bool found = dict_ids.find(dict_id) != dict_ids.end();
      printf("%2d%c %s\n", dict_id, found ? '*' : ':',
             dicts[dict_id]->info().c_str());
    }
  } else if (cmd[0] == "aliases") {
    traverse(aliases, alias) {
      printf("%s: %s\n",
             alias->first.c_str(),
             join(alias->second, ", ").c_str());
    }
  } else if (cmd[0] == "make") {
    if (cmd.size() >= 2) {
      if (cmd[1] == "toc") {
        traverse(current_dict_ids, current_dict_id) {
          Dict *dict = dicts[*current_dict_id];
          dict->make_toc();
        }
      } else if (cmd[1] == "macdic") {
        traverse(current_dict_ids, current_dict_id) {
          int limit = (cmd.size() >= 3) ? atoi(cmd[2].c_str()) : std::numeric_limits<int>::max();
          Dict *dict = dicts[*current_dict_id];
          dict->make_macdic_xml(limit, *current_dict_id);
        }
      } else if (cmd[1] == "sql") {
        sqlite3_sql_entry_id_ = 0;
        traverse(current_dict_ids, current_dict_id) {
          int limit = (cmd.size() >= 3) ? atoi(cmd[2].c_str()) : std::numeric_limits<int>::max();
          Dict *dict = dicts[*current_dict_id];
          dict->make_sqlite3_sql(limit, *current_dict_id);
        }
      } else if (cmd[1] == "henkakei") {
          // 変化形テーブルを作成。あとで関数名考える
        traverse(current_dict_ids, current_dict_id) {
          Dict *dict = dicts[*current_dict_id];
          dict->make_henkakei_table();
        }
      } else {
        printf("[command] make {toc|macdic}\n");
      }
    } else {
      printf("[command] make {toc|macdic}\n");
    }
  } else if (cmd[0] == "dump") {
    if (cmd.size() == 1) {
      printf("[command] dump {header|index|words|datablock <id>|all}\n");
    } else if (current_dict_ids.size() == 0) {
      printf("// 辞書が選択されていません。\n");
    } else {
      sqlite3_sql_entry_id_ = 0;
      traverse(current_dict_ids, current_dict_id) {
        Dict *dict = dicts[*current_dict_id];
        _dict = dict;
        PDICIndex *index = dict->index;
        std::string what_to_dump = cmd[1];
        if (what_to_dump == "header") {
          index->header->dump();
        } else if (what_to_dump == "index") {
          index->dump();
        } else if (what_to_dump == "datablock" && cmd.size() >= 3) {
          int ix = atoi(cmd[2].c_str());
          index->iterate_datablock(ix, &cb_dump_entry, NULL);
        } else if (what_to_dump == "words") {
          index->iterate_all_datablocks(&cb_dump_entry, NULL);
        } else if (what_to_dump == "examples") {
          index->iterate_all_datablocks(&cb_dump_examples, NULL);
        } else if (what_to_dump == "inline_examples") {
          index->iterate_all_datablocks(&cb_dump_inline_examples, NULL);
        } else if (what_to_dump == "macdic") {
          int limit = (cmd.size() >= 3) ? atoi(cmd[2].c_str()) : std::numeric_limits<int>::max();
          Dict *dict = dicts[*current_dict_id];
          dict->make_macdic_xml(limit, *current_dict_id);
        } else if (what_to_dump == "sql") {
          int limit = (cmd.size() >= 3) ? atoi(cmd[2].c_str()) : std::numeric_limits<int>::max();
          Dict *dict = dicts[*current_dict_id];
          dict->make_sqlite3_sql(limit, *current_dict_id);
        } else if (what_to_dump == "all") {
          index->iterate_all_datablocks(&cb_dump, NULL);
          // } else if (what_to_dump == "henkakei") {
          // index->iterate_all_datablocks(&cb_dump_eijiro_henkakei, NULL);
        } else {
          printf("// [ERROR] I don't know how to dump '%s'...\n",
                 what_to_dump.c_str());
        }
        _dict = NULL;
      }
    }
  } else if (cmd[0] == "lookup") {
    byte *needle = reinterpret_cast<byte*>(cmdstr) + 7;
    lookup(needle, params.default_lookup_flags);
  } else if (cmd[0] == "sarray") {
    byte *needle = reinterpret_cast<byte*>(cmdstr) + 7;
    lookup(needle, LOOKUP_SARRAY);
  } else if (cmd[0] == "regexp") {
    byte *needle = reinterpret_cast<byte*>(cmdstr) + 7;
    lookup(needle, LOOKUP_REGEXP);
  } else if (cmd[0] == "full") {
    byte *needle = reinterpret_cast<byte*>(cmdstr) + 5;
    lookup(needle, LOOKUP_FROM_ALL);
  } else if (cmd[0] == "clean") {
    free_all_cloned_buffers();
  } else {
    printf("// [ERROR] 未知のコマンド '%s' に遭遇...\n", cmd[0].c_str());
  }

  return true;
}

void Shell::do_alias(const std::string& alias, const std::string& valid_name) {
  std::vector<std::string> names(1, valid_name);
  do_alias(alias, names);
}

void Shell::do_alias(const std::string& alias,
              const std::vector<std::string>& valid_names) {
  aliases[alias] = valid_names;
}

std::vector<int> Shell::resolve_aliases(const std::string& name) {
  std::vector<int> dict_ids;

  if (nametable.find(name) != nametable.end()) {  // if found in nametable
    dict_ids.push_back(nametable[name]);
  } else if (aliases.find(name) != aliases.end()) {  // if found in aliases
    std::vector<std::string> names = aliases[name];
    traverse(names, name) {
      std::vector<int> ids = resolve_aliases(*name);
      dict_ids.insert(dict_ids.end(), ids.begin(), ids.end());
    }
  } else {
    printf("// [ERROR] %s が見つかりません。\n", name.c_str());
  }

  return dict_ids;
}

bool Shell::do_use(std::string name) {
  std::vector<int> dict_ids = resolve_aliases(name);

  if (dict_ids.size() > 0) {
    current_dict_ids.assign(dict_ids.begin(), dict_ids.end());
    current_dict_name = name;
    return true;
  } else {
    return false;
  }
}

void Shell::render_current_result() {
  int keep = params.render_count_limit;
  params.render_count_limit = std::numeric_limits<int>::max();
  traverse(current_result_vec, it) {
    render_result(*it, current_pattern);
  }
  params.render_count_limit = keep;
}

void Shell::render_current_result(const std::set<int>& range) {
  int keep = params.render_count_limit;
  params.render_count_limit = std::numeric_limits<int>::max();
  traverse(range, it) {
    render_result(current_result_vec[*it], current_pattern);
  }
  params.render_count_limit = keep;
}

void Shell::render_status() {
  printf("%s", ANSI_FGCOLOR_GREEN);

  printf("// verbose mode = %s\n",
         params.verbose_mode ? "ON" : "OFF");
  printf("// direct dump mode = %s\n",
         params.direct_dump_mode ? "ON" : "OFF");
  printf("// separator mode = %s\n",
         params.separator_mode ? "ON" : "OFF");
  printf("// full search mode = %s\n",
         params.full_search_mode ? "ON" : "OFF");
  printf("// ANSI coloring mode = %s\n",
         params.ansi_coloring_mode ? "ON" : "OFF");
  printf("// newline mode = %s\n",
         params.more_newline_mode ? "ON" : "OFF");
  printf("// default lookup mode = %s\n",
         params.get_lookup_mode());
  printf("// render count limit = %d, stop on limit = %s\n",
         params.render_count_limit,
         params.stop_on_limit_mode ? "ON" : "OFF");

  if (current_pattern) {
    printf("// 最後に行われた検索: (%s) \"%s\"\n",
           current_query.first.c_str(), current_query.second.c_str());
    // printf("// 最後に行われた検索: /"
    //  << current_pattern->pattern() << "/" << std::endl;
    printf("// 現在保持している結果: %d件\n",
           static_cast<int>(current_result_vec.size()));
  }
  printf("%s", ANSI_FGCOLOR_DEFAULT);
}

#define DEFAULT_RENDER_COUNT_LIMIT 150

ShellParams::ShellParams() {
  separator_mode       = false;
  verbose_mode         = false;
  direct_dump_mode     = false;
  full_search_mode     = false;
  ansi_coloring_mode   = false;
  more_newline_mode    = false;
  render_count_limit   = DEFAULT_RENDER_COUNT_LIMIT;
  stop_on_limit_mode   = true;

  default_lookup_flags = LOOKUP_PDIC_INDEX | LOOKUP_HENKAKEI
                                           | LOOKUP_EXACT_MATCH;
  debug_flags          = 0;
}

int ShellParams::set_render_count_limit(int limit) {
  if (limit)
    render_count_limit = limit;
  else
    render_count_limit = DEFAULT_RENDER_COUNT_LIMIT;
  return render_count_limit;
}

int ShellParams::set_lookup_mode(const char *mode_str) {
  if (strcmp(mode_str, "normal") == 0) {
    default_lookup_flags = LOOKUP_PDIC_INDEX | LOOKUP_HENKAKEI
                                             | LOOKUP_MATCH_FORWARD;
  } else if (strcmp(mode_str, "exact") == 0) {
    default_lookup_flags = LOOKUP_PDIC_INDEX | LOOKUP_HENKAKEI
                                             | LOOKUP_EXACT_MATCH;
  } else if (strcmp(mode_str, "forward") == 0) {
    default_lookup_flags = LOOKUP_PDIC_INDEX | LOOKUP_HENKAKEI
                                             | LOOKUP_MATCH_FORWARD;
  } else if (strcmp(mode_str, "sarray") == 0) {
    default_lookup_flags = LOOKUP_SARRAY | LOOKUP_HENKAKEI;
  } else if (strcmp(mode_str, "regexp") == 0) {
    default_lookup_flags = LOOKUP_REGEXP | LOOKUP_HENKAKEI;
  } else if (strcmp(mode_str, "all") == 0) {
    default_lookup_flags = LOOKUP_FROM_ALL;
  } else {
    return -1;
  }
  return 0;
}

const char *ShellParams::get_lookup_mode() {
  if (default_lookup_flags == (LOOKUP_PDIC_INDEX | LOOKUP_HENKAKEI
                                                 | LOOKUP_MATCH_FORWARD))
    return "normal";
  else if (default_lookup_flags == (LOOKUP_PDIC_INDEX | LOOKUP_HENKAKEI
                                                 | LOOKUP_EXACT_MATCH))
    return "exact";
  else if (default_lookup_flags == (LOOKUP_SARRAY | LOOKUP_HENKAKEI))
    return "sarray";
  else if (default_lookup_flags == (LOOKUP_REGEXP | LOOKUP_HENKAKEI))
    return "regexp";
  else
    return "all";
}
