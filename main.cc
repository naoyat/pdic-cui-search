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
#include "util.h"
#include "util_stl.h"
#include "bocu1.h"
#include "dump.h"
#include "charcode.h"

#include <ctime>
#include <sys/time.h>
timeval _timeval_start;
clock_t _clock_start;
void TIME_RESET() {
  _clock_start = clock();
  gettimeofday(&_timeval_start, NULL);
}
std::pair<int,int> TIME_USEC() {
  int timeval_usec, clock_usec;
  timeval timeval_end;
  gettimeofday(&timeval_end, NULL);
  clock_t clock_end = clock();
  timeval_usec = (timeval_end.tv_sec - _timeval_start.tv_sec)*1000000 + (timeval_end.tv_usec - _timeval_start.tv_usec);
  if (CLOCKS_PER_SEC == 1000000)
    clock_usec = clock_end - _clock_start;
  else
    clock_usec = (int)((long long)(clock_end - _clock_start) * 1000000 / CLOCKS_PER_SEC);

  return std::make_pair(timeval_usec, clock_usec);
}

#define rep(var,n)  for(int var=0;var<(n);var++)
#define traverse(c,i)  for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); i++)
#define all(c)  (c).begin(),(c).end()
#define found(s,e)  ((s).find(e)!=(s).end())

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
int calculate_space_for_index(PDICIndex *index);

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

  int from, to, cnt;
  cnt = index->bsearch_in_index(criteria->needle, exact_match, from, to);
#ifdef VERBOSE
  printf("lookup. from %d to %d, %d/%d...\n", from, to, cnt, index->_nindex);
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
//std::set<std::string> _words;
//std::priority_queue<std::string> _words;

int calculate_space_for_index(PDICIndex *index)
{
  TIME_RESET();

  _wordcount = 0;
  _wordsize_sum = 0;
  //_words.clear();
  
  index->iterate_all_datablocks(&count_word, NULL);

  std::pair<int,int> time = TIME_USEC();
  
  printf("%d words, %d bytes; %.2f b/w; real:%d process:%d.\n", _wordcount, _wordsize_sum, (double)_wordsize_sum/_wordcount, time.first, time.second);

  return _wordsize_sum;
}

void count_word(PDICDatafield *datafield)
{
  // Eijiro131: 1996426 words
  // set<string>: 5.2sec (= 2.6 usec/entry)
  // priority_queue<string>: 5.5sec (= 2.76 usec/entry)
  // e,jともにutf8文字列を用意: 3205 ms (= 1.6 usec/entry)
  // memcpyあり: 250 ms (= 0.125 usec/entry)
  // memcpyなし: 147 ms
  ++_wordcount;
  _wordsize_sum += (datafield->entry_index_size + 1);
}

