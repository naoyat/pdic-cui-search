// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/parse_lookup_result.h"

#include <map>
#include <string>
#include <vector>
#include <utility>

using namespace std;

pair<string, string> split_line(char *begin, char *end) {
  re2::StringPiece text(begin, end - begin);
  std::string eng, jap;
  if (RE2::FullMatch(text, "([ -~]+)(.*)", &eng, &jap)) {
    return make_pair(eng, jap);
  } else {
    return make_pair(text.as_string().c_str(), "");
  }
}

pair<string, string> parse_usage(char **begin, char *end) {
  char *next_begin = static_cast<char*>(memchr(*begin, '\r', end - *begin));
  end = next_begin ? next_begin : end;
  next_begin = next_begin ? next_begin+2 : end;

  *begin += 3;
  char *remark_begin = strnstr(*begin, "◆", end - *begin);
  char *remark_end = end;
  if (remark_begin) {
    end = remark_begin;
    remark_begin += 3;
  } else {
    remark_begin = end;
  }

  pair<string, string> usage = split_line(*begin, end);

  // if (alt_show_usage)
  // if (alt_show_remark)
  std::string remark(remark_begin, remark_end - remark_begin);
  // printf(" (%s)", remark.c_str());

  *begin = next_begin;

  return usage;
}

pair<string, int> parse_pos(char **begin, char *end) {
  // if (strncmp(begin, "【", 3) == 0) return make_pair("", 0);
  char *pos_begin = *begin + 3;
  char *pos_end = strnstr(pos_begin, "】", end - pos_begin);
  if (pos_end != NULL) {
    *begin = pos_end + 3;
    if ('1' <= *pos_begin && *pos_begin <= '9') {
      if (pos_end == pos_begin+1) {
        int sub_id = atoi(pos_begin);
        return make_pair("*", sub_id);
      } else {
        // 1-動, 2-名 などの 1-, 2- を無視してみる
        char *hyphen = static_cast<char*>(
            memchr(pos_begin+1, '-', pos_end - (pos_begin+1)));
        if (hyphen) pos_begin = hyphen+1;
      }
    }

    // pos: line+3 .. end
    char *sub = static_cast<char*>(
        memchr(pos_begin+1, '-', pos_end - (pos_begin+1)));
    string pos;
    int sub_id;
    if (sub) {
      pos = string(pos_begin, sub - pos_begin);
      sub_id = atoi(sub+1);
    } else {
      pos = string(pos_begin, pos_end - pos_begin);
      sub_id = 0;
    }
    return make_pair(pos, sub_id);
  }
  return make_pair("", 0);
}

vector<string> parse_line(char **begin, char *end) {
  if (end == *begin) return vector<string>();

  vector<string> meanings;
  char *rest_begin = *begin;
  char *rest_end   = end;

  char *remark_begin = strnstr(rest_begin, "◆", rest_end - rest_begin);
  char *remark_end = rest_end;
  if (remark_begin) {
    rest_end = remark_begin;
    remark_begin += 3;
  }

  char *next_begin = strnstr(rest_begin, "【", rest_end - rest_begin);
  if (next_begin) {
    *begin = rest_end = next_begin;
  } else {
    *begin = end;
  }

  if (rest_end > rest_begin) {
    std::string rest(rest_begin, rest_end - rest_begin);
    meanings.push_back(rest);
  }

  if (remark_begin) {
    std::string remark(remark_begin, remark_end - remark_begin);
    meanings.push_back(remark);
  }

  return meanings;
}

pair< map<string, meanings_t>, vector<usage_t> > parse_jword(byte* jword) {
  map<string, meanings_t> meanings_map;
  vector<usage_t> usages;

  string pos = "";
  int sub_id = 0;

  char *data_begin = reinterpret_cast<char*>(jword);
  char *data_end = data_begin + strlen(data_begin);

  char *begin = data_begin;
  while (begin < data_end) {
    // if (0 <= *begin && *begin < 0x20) ++begin;
    char *newline = static_cast<char*>(memchr(begin, '\r', data_end - begin));
    char *end = newline ? newline : data_end;
    char *next_begin = newline ? newline+2 : data_end;

    while (begin < end) {
      if (strncmp(begin, "【", 3) == 0) {
        pair<string, int> pos_ = parse_pos(&begin, end);
        if (pos_.first != "") {
          pos    = pos_.first;
          sub_id = pos_.second;
        }

        vector<string> meanings = parse_line(&begin, end);
        while (0 <= *begin && *begin < 0x20) ++begin;
        if (meanings.size() > 0) {
          if (meanings_map.find(pos) == meanings_map.end()) {
            meanings_map[pos] = meanings_t();
          }
          meanings_map[pos].push_back(make_pair(sub_id, meanings[0]));
          // !!! "◆" 以降の付加情報(meanings[1])を捨てている
        }
      } else if (strncmp(begin, "・", 3) == 0) {
        pair<string, string> usage = parse_usage(&begin, end);
        while (0 <= *begin && *begin < 0x20) ++begin;

        usages.push_back(make_pair(make_pair(pos, sub_id), usage));
      } else {
        vector<string> meanings = parse_line(&begin, end);
        while (0 <= *begin && *begin < 0x20) ++begin;
        if (meanings.size() > 0) {
          if (meanings_map.find("*") == meanings_map.end()) {
            meanings_map["*"] = meanings_t();
          }
          meanings_map["*"].push_back(make_pair(0, meanings[0]));
        }
        // printf("？%s",(byte)begin[0],(byte)begin[1],(byte)begin[2],begin);
        // meanings_map[""].push_back(make_pair(0, string(begin, end-begin)));
        // break;
      }
    }

    if (!next_begin) break;

    begin = next_begin;
    while (0 <= *begin && *begin < 0x20) ++begin;
  }

  return make_pair(meanings_map, usages);
}
