#include "Dict.h"

#include <vector>

#include <libgen.h>
#include <strings.h>

#include "PDICHeader.h"
#include "PDICIndex.h"
#include "PDICDatafield.h"
#include "filemem.h"
#include "timeutil.h"
#include "util_stl.h"

#ifdef DEBUG
#include "cout.h"
#endif

extern std::vector<std::string> loadpaths;
extern bool verbose_mode;

Dict::Dict(FILE *fp, const std::string& name, const std::string& path)
{
  this->fp = fp;
  this->index = new PDICIndex(fp);
  this->name = name;
  this->path = path;

  this->entry_buf = (byte *)NULL;
  this->entry_start_pos = (int *)NULL;
  this->entry_suffix_array = (int *)NULL;

  _suffix = new char[name.size()+1];
  strcpy(_suffix, basename((char *)name.c_str()));
  int len = strlen(_suffix);
  if (strcasecmp(_suffix + len-4, ".dic") == 0) {
    _suffix[len - 4] = 0;
  }

  this->load_additional_files();
}

Dict::~Dict()
{
  fclose(fp);

  delete _suffix;
  delete index;

  unload_additional_files();
}

// global (only for callback routines)
byte *_entry_buf = NULL; // 全ての見出し語（に'\0'を付加したデータ）を結合したもの
int _entry_buf_offset = 0;
std::vector<int> _entry_start_pos; // 各見出し語の開始位置(entry_buf + entry_start_pos[i])

void stock_entry_words(PDICDatafield *datafield)
{
  byte *entry_utf8 = datafield->entry_word_utf8();
  int entry_utf8_memsize = strlen((char *)entry_utf8) + 1;
  if (_entry_buf_offset + entry_utf8_memsize > ENTRY_BUF_SIZE) {
    printf("memory over at #%d (%s)\n", (int)_entry_start_pos.size(), entry_utf8);
    return;
  }
  _entry_start_pos.push_back(_entry_buf_offset);
  memcpy(_entry_buf + _entry_buf_offset, entry_utf8, entry_utf8_memsize); // returns the position at \0
  _entry_buf_offset += entry_utf8_memsize;
}

int
Dict::make_sarray_index(int buffer_size_enough_for_whole_entries)
{
  printf("now making suffix array index...\n");
  time_reset();

  // initialize
  _entry_buf = (byte *)malloc(buffer_size_enough_for_whole_entries);
  if (!_entry_buf) {
    printf("entry_buf is not allocated\n");
    return -1;
  }
  _entry_start_pos.clear();
  _entry_buf_offset = 0;

  (_entry_buf)[_entry_buf_offset++] = 0;

  index->iterate_all_datablocks(&stock_entry_words, NULL);

  std::pair<int,int> time = time_usec();
  int entry_buf_size = _entry_buf_offset;
  printf("%d words, %d bytes; real:%d process:%d.\n", (int)_entry_start_pos.size(), entry_buf_size, time.first, time.second);

  time_reset();

  std::pair<int*,int> sarr = make_light_suffix_array(_entry_buf, entry_buf_size);
  int *suffix_array = sarr.first;
  int suffix_array_length = sarr.second;

  int *entry_start_pos = (int *)malloc(sizeof(int)*_entry_start_pos.size());
  if (!entry_start_pos) {
    printf("entry_start_pos is not allocated\n");
    return -1;
  }
  for (int i=0,c=_entry_start_pos.size(); i<c; ++i) entry_start_pos[i] = _entry_start_pos[i];

  std::string savepath = this->suffix();
  savemem((savepath + SUFFIX_ENTRY).c_str(), _entry_buf, entry_buf_size);
  savemem((savepath + SUFFIX_ENTRY_START).c_str(), (byte *)entry_start_pos, sizeof(int)*_entry_start_pos.size());
  savemem((savepath + SUFFIX_ENTRY_SARRAY).c_str(), (byte *)suffix_array, sizeof(int)*suffix_array_length);

  free((void *)_entry_buf); _entry_buf = NULL;
  free((void *)entry_start_pos);
  free((void *)suffix_array);
  _entry_start_pos.clear();

  this->load_additional_files();

  std::pair<int,int> time2 = time_usec();
  printf("%d/%d ; real:%d process:%d.\n", this->entry_suffix_array_length, _entry_buf_offset, time2.first, time2.second);

  return entry_buf_size;
}

std::vector<int>
Dict::search_in_sarray(byte *needle)
{
  search_result_t result = search(entry_buf, entry_suffix_array, entry_suffix_array_length, needle, false);
  if (verbose_mode) {
    printf("SARRAY: "); std::cout << result << std::endl;
  }

  std::set<int> matched_offsets;
  /*      
  if (result.second.first >= 0) {
    printf(" ? %s\n", this->entry_buf + entry_suffix_array[result.second.first]);
  }
  */
  if (result.first) {
    for (int i=result.second.first; i<=result.second.second; ++i) {
      byte *head = strhead(this->entry_buf + entry_suffix_array[i]);
      int offset = (int)(head - this->entry_buf);
      matched_offsets.insert(offset);
    }
  }

  return std::vector<int>(all(matched_offsets));
}

void
Dict::unload_additional_files()
{
  if (entry_buf) {
    unloadmem((byte *)entry_buf);
    entry_buf = NULL;
  }
  if (entry_start_pos) {
    unloadmem((byte *)entry_start_pos);
    entry_start_pos = (int *)NULL;
  }
  if (entry_suffix_array) {
    unloadmem((byte *)entry_suffix_array);
    entry_suffix_array = (int *)NULL;
  }
}

bool
Dict::load_additional_files()
{
  unload_additional_files();

  for (int i=0; i<loadpaths.size(); ++i) {
    std::string path = loadpaths[i] + "/" + this->suffix();

    this->entry_buf = (byte *)loadmem((path + SUFFIX_ENTRY).c_str());
    if (this->entry_buf) {

      this->entry_start_pos = (int *)loadmem((path + SUFFIX_ENTRY_START).c_str());
      this->entry_start_rev.clear();
      if (this->entry_start_pos) {
        this->entry_start_pos_length = mem_size((byte *)this->entry_start_pos) / sizeof(int);
        for (int i=0; i<this->entry_start_pos_length; ++i) {
          entry_start_rev[ this->entry_start_pos[i] ] = i;
        }
      }
      this->entry_suffix_array = (int *)loadmem((path + SUFFIX_ENTRY_SARRAY).c_str());
      if (this->entry_suffix_array) {
        this->entry_suffix_array_length = mem_size((byte *)this->entry_suffix_array) / sizeof(int);
      }
      if (verbose_mode) {
        std::cout << this->suffix() << ": loaded sarray index successfully." << std::endl;
      }
      return true;
    }
  }
  return false;
}
