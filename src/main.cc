#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatablock.h"
#include "PDICDatafield.h"
#include "Criteria.h"
#include "util.h"
#include "util_stl.h"
#include "bocu1.h"
#include "dump.h"
#include "charcode.h"
#include "timeutil.h"
//#include "bsearch.h"
#include "suffix_array.h"

#define DEBUG
#define VERBOSE

#ifdef DEBUG
#include "cout.h"
#endif

class Dict {
 public:
  FILE *fp;
  PDICIndex *index;
  std::string path, name;
 public:
  Dict(FILE *fp, const std::string& name, const std::string& path) {
    this->fp = fp;
    this->index = new PDICIndex(fp);
    this->name = name;
    this->path = path;
  }
  ~Dict() { fclose(fp); delete index; }
  std::string info() { return name + " " + path; }
};

// prototypes
void load_rc();
std::vector<int> resolve_aliases(const std::string& name);
void do_use(std::string name);
void do_lookup(char *needle, int needle_len=0);
void lookup(FILE *fp, PDICIndex *index, unsigned char *needle, bool exact_match);

void dump(PDICDatafield *datafield);
void dump_word(PDICDatafield *datafield);
void count_word(PDICDatafield *datafield);
void stock_entry_words(PDICDatafield *datafield);

int calculate_space_for_index(PDICIndex *index);
int make_index(PDICIndex *index);


int do_load(const std::string& filename);
void do_alias(const std::string& alias, const std::string& valid_name);
void do_alias(const std::string& alias, const std::vector<std::string>& valid_names);
bool do_command(char *cmdstr);

// globals
std::vector<std::string> loadpaths;
std::vector<Dict*> dicts;
std::map<std::string,std::vector<std::string> > aliases; // name -> name
std::map<std::string,int> nametable; // name -> dict_id

std::vector<int> current_dict_ids;
std::string current_dict_name = "";

int main(int argc, char **argv)
{
  load_rc();
  
  if (argc >= 2) {
    std::string filename = argv[1];
    do_load(filename);
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

  traverse(dicts, dict) delete *dict;

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
#ifdef VERBOSE
  std::cout << "LOOKUP: " << current_dict_name << " " << current_dict_ids << std::endl;
#endif
  traverse(current_dict_ids, current_dict_id) {
    Dict *dict = dicts[*current_dict_id];
    bool exact_match = true;
    if (needle[needle_len-1] == '*') {
      exact_match = false;
      needle[needle_len-1] = 0;
    }
    lookup(dict->fp, dict->index, (unsigned char *)needle, exact_match);
  }
}

void lookup(FILE *fp, PDICIndex *index, unsigned char *needle, bool exact_match)
{
  int target_charcode = index->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS;
  
  Criteria *criteria = new Criteria(needle, target_charcode, exact_match);

  std::pair<int,int> range = index->bsearch_in_index(criteria->needle, exact_match);
  int from = range.first, to = range.second;
#ifdef VERBOSE
  printf("lookup. from %d to %d, %d/%d...\n", from, to, to-from+1, index->_nindex);
#endif
  for (int ix=from; ix<=to; ix++) {
    if (ix < 0) continue;
    if (ix >= index->_nindex) break;

    PDICDatablock* datablock = new PDICDatablock(fp, index, ix);
    datablock->iterate(&dump, criteria);
    //datablock->iterate(&dump, NULL);
    delete datablock;
  }
}

void dump(PDICDatafield *datafield)//unsigned char *entry, unsigned char *jword)
{
  puts((char *)datafield->entry_word_utf8());

  unsigned char *jword_indented = (unsigned char *)indent((char *)"   ", (char *)datafield->jword_utf8());
  puts((char *)jword_indented);
  free((char *)jword_indented);
}
void dump_word(PDICDatafield *datafield)
{
  puts((char *)datafield->entry_word_utf8());
  //puts((char *)entry);
}

int do_load(const std::string& filename)
{
  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + filename;

    FILE *fp = fopen(path.c_str(), "r");
    if (fp != NULL) {
      int new_dict_id = dicts.size();

      dicts.push_back( new Dict(fp, filename, path) );
      nametable[filename] = new_dict_id;

      //#ifdef VERBOSE
      printf("loading %s... => { name: %s, dict_id: %d }\n", path.c_str(), filename.c_str(), new_dict_id);
      //#endif
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
#ifdef VERBOSE
        std::cout << "+" << filename << std::endl;
#endif
        for (int i=2; i<cmd.size(); ++i) {
          do_alias(cmd[i], filename);
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
    set<int> dict_ids(all(current_dict_ids));
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
  else if (cmd[0] == "dump") {
    if (cmd.size() >= 2) {
      if (current_dict_ids.size() == 0) {
        printf("ERROR: no dictionary selected\n");
      } else {
#ifdef VERBOSE
        std::cout << "DUMP: " << current_dict_name << " " << current_dict_ids << std::endl;
#endif
        traverse(current_dict_ids, current_dict_id) {
          PDICIndex *index = dicts[*current_dict_id]->index;
          std::string what_to_dump = cmd[1];
          if (what_to_dump == "header") {
            index->header->dump();
          } else if (what_to_dump == "index") {
            index->dump();
          } else if (what_to_dump == "words") {
            index->iterate_all_datablocks(&dump_word, NULL);
          } else if (what_to_dump == "count") {
            printf("[%d]", *current_dict_id);
            calculate_space_for_index(index);
          } else if (what_to_dump == "all") {
            index->iterate_all_datablocks(&dump, NULL);
          } else {
            printf("ERROR: I don't know how to dump '%s'...\n", what_to_dump.c_str());
          }
        }
      }
    } else {
      printf("[command] dump {header|index|words|all}\n");
    }
  }
  else if (cmd[0] == "dev") {
    if (cmd.size() >= 2) {
      if (current_dict_ids.size() == 0) {
        printf("ERROR: no dictionary selected\n");
      } else {
#ifdef VERBOSE
        std::cout << "DUMP: " << current_dict_name << " " << current_dict_ids << std::endl;
#endif
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
  else if (cmd[0] == "lookup") {
    do_lookup(cmdstr + 7);
    newline();
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

unsigned char *entry_buf = NULL; // 全ての見出し語（に'\0'を付加したデータ）を結合したもの
int entry_buf_offset = 0;
vector<int> entry_start_pos; // 各見出し語の開始位置(entry_buf + entry_start_pos[i])
#define ENTRY_BUF_SIZE 1024*1024*128 // 128MBとりあえず

int make_index(PDICIndex *index)
{
  time_reset();

  // initialize
  entry_buf = (unsigned char *)malloc(ENTRY_BUF_SIZE);
  if (!entry_buf) {
    printf("entry_buf is not allocated\n");
    return -1;
  }
  entry_start_pos.clear();
  entry_buf_offset = 0;

  index->iterate_all_datablocks(&stock_entry_words, NULL);

  std::pair<int,int> time = time_usec();
  printf("%d words, %d bytes; real:%d process:%d.\n", (int)entry_start_pos.size(), entry_buf_offset, time.first, time.second);

  time_reset();

  std::pair<int*,int> sarr = make_light_suffix_array(entry_buf, entry_buf_offset);
  int* offsets = sarr.first;
  int offsets_len = sarr.second;

  unsigned char *needle = (unsigned char *)"whose creativity";
  search_result_t result = search(entry_buf, offsets, offsets_len, needle, false);
  printf("RESULT: "); std::cout << result << std::endl;
  if (result.second.first >= 0) {
    printf(" ? %s\n", entry_buf+offsets[result.second.first]);
  }
  if (result.first) {
    for (int i=result.second.first; i<=result.second.second; ++i) {
      printf(" - %s\n", entry_buf+offsets[i]);
    }
  } else {
    printf(" - (not found)\n");
  }
  free((void *)offsets);

  std::pair<int,int> time2 = time_usec();
  printf("%d/%d ; real:%d process:%d.\n", offsets_len, entry_buf_offset, time2.first, time2.second);

  return entry_buf_offset;
}

void stock_entry_words(PDICDatafield *datafield)
{
  unsigned char *entry_utf8 = datafield->entry_word_utf8();
  int entry_utf8_memsize = strlen((char *)entry_utf8) + 1;
  if (entry_buf_offset + entry_utf8_memsize > ENTRY_BUF_SIZE) {
    printf("memory over at #%d (%s)\n", (int)entry_start_pos.size(), entry_utf8);
    return;
  }
  entry_start_pos.push_back(entry_buf_offset);
  memcpy(entry_buf + entry_buf_offset, entry_utf8, entry_utf8_memsize); // returns the position at \0
  entry_buf_offset += entry_utf8_memsize;
}

