#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatablock.h"
#include "util.h"
#include "bocu1.h"

#include <string>
#include <vector>
#include <map>
//using namespace std;

void dump(unsigned char *entry, unsigned char *jword)
{
  printf("%s\n", entry);

  unsigned char *jword_indented = (unsigned char *)indent((char *)"   ", (char *)jword);
  printf("%s\n", jword_indented);
  free((char *)jword_indented);
}

void lookup(FILE *fp, PDICIndex *index, unsigned char *needle, bool exact_match)
{
  int from, to, cnt;
  cnt = index->bsearch_in_index((unsigned char *)needle, exact_match, from, to);
  //printf("bsearch \"%s\": %d [%d..%d]\n", needle, cnt, from, to);
  // for (int ix=from-1; ix<=to+1; ix++) {

  Criteria *criteria = new Criteria(needle, exact_match);
  
  for (int ix=from; ix<=to; ix++) {
    if (ix < 0) continue;
    if (ix >= index->nindex) break;

    //unsigned char *entry_word = bocu1_to_utf8(index->entry_words[ix]);
    //printf(" %c %d:\"%s\".\n", (from <= ix && ix <= to ? '=' : 'x'), ix, entry_word);
    //free(entry_word);

    PDICDatablock* datablock = new PDICDatablock(fp, index, ix);
    //datablock->iterate(&needle_bocu1, exact_match, &dump);
    datablock->iterate(&dump, criteria);
    delete datablock;
  }
  newline();
}

int main(int argc, char **argv)
{
  std::vector<std::pair<FILE*,PDICIndex*> > dicts;
  int current_dict_id = -1;
  std::string current_dict_name;

  std::map<std::string,int> names; // name -> dict_id

  if (argc == 2) {
    char *fname = argv[1];

    FILE *fp = fopen(fname, "r");
    if (fp != NULL) {
      int new_dict_id = 0;
      std::string new_dict_name = fname;
      dicts.push_back( std::make_pair(fp, new PDICIndex(fp)) );
      names[new_dict_name] = new_dict_id;

      current_dict_id = new_dict_id;
      current_dict_name = fname;
    }
  }

  // REPL
  for (bool looping=true; looping; ) {
    char line[256];
    printf("%s> ", current_dict_name.c_str());
    if (!fgets(line, 256, stdin)) { newline(); break; }
    int linelen = strlen(line); line[--linelen] = 0;
    if (linelen == 0) continue;

    switch (line[0]) {
      case '.': { // command mode
        char *cmd = line+1;
        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "bye") == 0) {
          printf("bye.\n"); looping = false; break;
        }
        else if (strncmp(cmd, "load ", 5) == 0) {
          char *path = cmd + 5;
          printf("loading %s... \n", path);
          FILE *fp = fopen(path, "r");
          if (fp != NULL) {
            int new_dict_id = dicts.size();
            std::string new_dict_name = path;
            dicts.push_back( std::make_pair(fp, new PDICIndex(fp)) );
            names[new_dict_name] = new_dict_id;

            current_dict_id = new_dict_id;
            current_dict_name = new_dict_name;
          }
        }
        else if (strncmp(cmd, "dump ", 5) == 0) {
          if (current_dict_id >= 0) {
            PDICIndex *index = dicts[current_dict_id].second;
            char *what_to_dump = cmd + 5;
            if (strcmp(what_to_dump, "header") == 0) {
              index->header->dump();
            } else if (strcmp(what_to_dump, "index") == 0) {
              index->dump();
            } else {
              printf("I don't know how to dump '%s'...\n", what_to_dump);
            }
          } else if (current_dict_id < 0) {
            printf("no dictionary selected\n");
          }
        } else {
          printf("UNKNOWN COMMAND, '%s' %d\n", line, linelen);
        }
      } break;

      default: {
        if (current_dict_id >= 0) {
          std::pair<FILE*,PDICIndex*> dict = dicts[current_dict_id];
          bool exact_match = true;
          if (line[linelen-1] == '*') {
            exact_match = false;
            line[linelen-1] = 0;
          }
          //printf("lookup now \"%s\" in (%d,%s)...\n", line, current_dict_id, current_dict_name.c_str());
          lookup(dict.first, dict.second, (unsigned char *)line, exact_match);
        } else {
          printf("no dictionary selected\n");
        }
      } break;
    }
  }

  for (int dict_id=0; dict_id<dicts.size(); dict_id++) {
    std::pair<FILE*,PDICIndex*> dict = dicts[dict_id];
    fclose(dict.first);
    delete dict.second;
  }
  
  return 0;
}
