#include "Dict.h"

#include <vector>

#include <libgen.h>
#include <strings.h>

#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatafield.h"

#include "timeutil.h"

#ifdef DEBUG
#include "cout.h"
#endif

Dict::Dict(FILE *fp, const std::string& name, const std::string& path)
{
  this->fp = fp;
  this->index = new PDICIndex(fp);
  this->name = name;
  this->path = path;

  _suffix = new char[name.size()+1];
  strcpy(_suffix, basename((char *)name.c_str()));
  int len = strlen(_suffix);
  if (strcasecmp(_suffix + len-4, ".dic") == 0) {
    _suffix[len - 4] = 0;
  }
}

Dict::~Dict()
{
  fclose(fp);

  delete index;
  delete _suffix;
}

byte *entry_buf = NULL; // 全ての見出し語（に'\0'を付加したデータ）を結合したもの
int entry_buf_offset = 0;
std::vector<int> entry_start_pos; // 各見出し語の開始位置(entry_buf + entry_start_pos[i])
#define ENTRY_BUF_SIZE 1024*1024*128 // 128MBとりあえず

void stock_entry_words(PDICDatafield *datafield)
{
  byte *entry_utf8 = datafield->entry_word_utf8();
  int entry_utf8_memsize = strlen((char *)entry_utf8) + 1;
  if (entry_buf_offset + entry_utf8_memsize > ENTRY_BUF_SIZE) {
    printf("memory over at #%d (%s)\n", (int)entry_start_pos.size(), entry_utf8);
    return;
  }
  entry_start_pos.push_back(entry_buf_offset);
  memcpy(entry_buf + entry_buf_offset, entry_utf8, entry_utf8_memsize); // returns the position at \0
  entry_buf_offset += entry_utf8_memsize;
}

int
Dict::make_sarray_index()
{
  time_reset();

  // initialize
  entry_buf = (byte *)malloc(ENTRY_BUF_SIZE);
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

#ifdef DEBUG
  byte *needle = (byte *)"whose creativity";
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
#endif

  free((void *)offsets);

  std::pair<int,int> time2 = time_usec();
  printf("%d/%d ; real:%d process:%d.\n", offsets_len, entry_buf_offset, time2.first, time2.second);

  return entry_buf_offset;
}

int
Dict::load_sarray_index()
{
}
