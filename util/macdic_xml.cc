// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "util/macdic_xml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <re2/re2.h>
//#include <re2/stringpiece.h>

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip> // setfill
// using namespace std;

#include "pdic/Dict.h"
#include "pdic/PDICDatafield.h"
#include "util/stlutil.h"

extern int dump_remain_count_;
extern int current_dict_id_;

FILE *macdic_xml_fp = NULL;
int macdic_xml_entry_id = 0;

int trap_mode_ = 0;

// static
std::string idstr(const char *cstr);
char *escape(const char *str);
void output(const char *begin, const char *end=NULL);
void parse_output(const char *begin, const char *end=NULL);
void render_definition(const char *wordclass, const char *desc);
// int macdic_xml_open(std::string path);
// void macdic_xml_close();
// void cb_macdic_xml(PDICDatafield *datafield);

int macdic_xml_open(std::string path) {
  printf("[%s]", path.c_str());

  macdic_xml_fp = fopen(path.c_str(), "w");
  if (!macdic_xml_fp) return -1;

  macdic_xml_entry_id = 0;
  return 0;
}

void macdic_xml_close() {
  fclose(macdic_xml_fp);
  macdic_xml_fp = NULL;

  printf(" macdic_xml_entry_id = %d\n", macdic_xml_entry_id);
}

void cb_macdic_xml(PDICDatafield *datafield) {
  byte *entry    = datafield->in_utf8(F_ENTRY);
  byte *jword    = datafield->in_utf8(F_JWORD);
  // byte *example  = datafield->in_utf8(F_EXAMPLE); // unused
  byte *pron     = datafield->in_utf8(F_PRON);
  int   level    = datafield->entry_word_level;

  if (!entry) return;
  if (!jword) return;
  ///if (!pron) return; /// DEBUG

  //if (!(entry[0] == 'a' || entry[0] == 'A' || strncmp("あ", (const char *)entry, 3)==0)) return;
  //if (!(entry[0] == 'r' || entry[0] == 'R')) return;
  //if (level == 0) return;
  
  if (trap_mode_) {
    printf("# TRAP(%s)(%s)(%s)\n", entry, jword, pron);
  }
  /*
  if (strcmp((const char *)entry,
    
             "本当に？"
             // "本当に2台も冷蔵庫が必要なのかな？"
             // "本当に,誰かが私に日本の化粧室でのエチケットを教えてくれていたらよかったのにと思います"
             ) == 0) trap_mode_ = 1;
  */
  // if (strcmp((const char *)entry, "time cream") == 0) trap_mode_ = 1;

  macdic_xml_entry_id++;
  --dump_remain_count_;

  int jl = strlen((char *)jword);
  // std::vector<int> items;
  if (trap_mode_) printf("#   jl = %d\n", jl);

  // items.push_back(0);
  //const char *p = (const char *)jword, *q;
  using namespace std;
  vector<pair<int, int> > offsets;

  const char *p0 = (const char *)jword, *pE = p0 + jl;
  if (trap_mode_) printf("#   (p0, pE) = %p, %p\n", p0, pE);

  for (const char *p=p0; p<pE; ) {
    if (trap_mode_) printf("#   %d/%d\n", (int)(p-p0), (int)(pE-p0));
    // printf("..%d/%d", p-p0, jl);
    const char *q1 = strstr(p, "【");
    if (!q1) break;
    if (q1 >= p0+3 && strncmp(q1-3, "◆", 3) == 0) {
      p = q1 + 3; continue;
    }
    const char *q2 = strstr(q1+3, "】");
    if (!q2) break;
    offsets.push_back(make_pair(q1-p0, (q2+3)-p0));
    //p = q1 + 9;
    p = q2 + 3;
  }

  int n = (int)offsets.size();
  if (trap_mode_) {
    printf("#   n = %d\n", n);
    cout << "#   " << offsets << endl;
  }

  vector<pair<string, string> > items;
  vector<pair<string, string> > items_to_prepend;

  set<string> conj_forms;
  // vector<string> conj_forms;

  if (trap_mode_) printf("#   B %d\n", n);

  //printf("\n");
  bool prepend_mode = false;
  if (n > 0) {
    // 【】あり
    offsets.push_back(make_pair(jl,0));
    
    for (int i=0; i<n; i++) {
      int j1 = offsets[i].first, j2 = offsets[i].second;
      int j1next = offsets[i+1].first;
      if (trap_mode_) printf("#   B/true %d (%d %d) %d\n", i, j1, j2, j1next);

      int s1_from = j1 + 3, s1_len = j2 - j1 - 6;
      int s2_from = j2,     s2_len = j1next - j2;
      if (trap_mode_) printf("#   B/true [%d+%d], [%d+%d]\n", s1_from, s1_len, s2_from, s2_len);
      
      if (p0[s2_from + s2_len - 2] == 0x0d) s2_len -= 2;
      // if (p0[s2_from + s2_len - 1] == 0x0a) s2_len -= 1;
      // if (p0[s2_from + s2_len - 1] == '\n') --s2_len;
      if (strncmp("、", p0 + s2_from + s2_len - 3, 3) == 0) s2_len -= 3;

      if (trap_mode_) printf("#   B/true 3\n");
      
      string s1(p0 + s1_from, s1_len);
      string s2(p0 + s2_from, s2_len);
      
      if (trap_mode_) printf("#   B/true 4\n");

      if (s1 == "＠") prepend_mode = true;
      if (prepend_mode) {
        items_to_prepend.push_back(make_pair(s1, s2));
      } else {
        items.push_back(make_pair(s1, s2));
      }

      if (trap_mode_) printf("#   B/true 5\n");

      if (s1 == "変化") {
        string s2a(s2.begin(), s2.end());
        // 変化形を分解する
        RE2::GlobalReplace(&s2a, " または ", " | ");
        RE2::GlobalReplace(&s2a, "過去・過分＝", "");
        RE2::GlobalReplace(&s2a, "〕", "");
        RE2::GlobalReplace(&s2a, "、", "");
        RE2::GlobalReplace(&s2a, "《[^A-Za-z]*》", "|");
        RE2::GlobalReplace(&s2a, " \\| ", "|");
        RE2::GlobalReplace(&s2a, ";", "|");
        RE2::GlobalReplace(&s2a, "\\r\\n.*$", "");
        s2a = s2a + "|";
        printf("<変化> \"%s\"\n", s2a.c_str());

        re2::StringPiece input(s2a);
        string var;

        // while (RE2::Consume(&input, "([A-Za-z][-a-z()]*)|", &var)) {
        while (RE2::Consume(&input, "([^|]*)\\|", &var)) {
          if (var == "") continue;
          string pre, paren, post;
          if (RE2::FullMatch(var, "([-A-Za-z]*)\\(([^)]*)\\)([-A-Za-z]*)", &pre, &paren, &post)) {
            printf("  - %s%s%s | %s%s\n",
                   pre.c_str(), paren.c_str(), post.c_str(),
                   pre.c_str(), post.c_str()
                   );
            conj_forms.insert(pre + paren + post);
            conj_forms.insert(pre + post);
          } else {
            printf("  - %s\n", var.c_str());
            conj_forms.insert(var);
          }
        }
      }
      // printf("#%d: ([%d-%d] %d) %s | %s\n", i, j1, j2, j1next, s1.c_str(), s2.c_str());
    }
  } else {
    if (trap_mode_) printf("#   B/alt %s %d\n", (const char *)jword, jl);
    // 【】なし
    string s2((const char *)jword, jl);
    items.push_back(make_pair("", s2));
    // printf("##: ([] %d) --- | %s\n", jl, s2.c_str());
  }

  if (trap_mode_) printf("#   C\n");

  std::string id = idstr((const char *)entry);
  if (trap_mode_) printf("#   id = \"%s\"\n", id.c_str());

  fprintf(macdic_xml_fp, "\n"); // spacing
  // entryをescapeしないと (" -> &quot; とか)
  char *escaped_entry = escape((const char *)entry);
  fprintf(macdic_xml_fp,
          "<d:entry id=\"%s\" d:title=\"%s\">\n",
          id.c_str(), escaped_entry);
  fprintf(macdic_xml_fp,
          "\t<d:index d:value=\"%s\" />\n",
          escaped_entry);

  // ハッシュに格納してある変化形でも引けるようにする
  if (conj_forms.size() > 0) {
    // sort(conj_forms.begin(), conj_forms.end());

    for (typeof(conj_forms.begin()) it=conj_forms.begin(); it!=conj_forms.end(); ++it) {
      // for (int i=0; i<(int)conj_forms.size(); ++i) {
      // const char *elem = conj_forms[i].c_str();
      const char *elem = it->c_str();
      // if (elem == entry) continue;
      if (strcmp(elem, (const char *)entry) == 0) continue;

      fprintf(macdic_xml_fp,
              "\t<d:index d:value=\"%s\" " "d:title=\"%s (%s)\" />\n",
              elem,
              elem, escaped_entry);
    }
  }

  if (trap_mode_) printf("#   H1= %s\n", escaped_entry);
  
  // H1= entry
  fprintf(macdic_xml_fp, "\t<h1>%s</h1>\n", escaped_entry);
  if (pron) {
    fprintf(macdic_xml_fp,
            "\t<span class=\"syntax\"><span class=\"pr\">/ %s /</span></span>\n",
            pron);
  }

  // definition
  fprintf(macdic_xml_fp, "\t<p>");

  // cout << "P:" << items_to_prepend << endl;
  if (items_to_prepend.size() > 0) {
    for (int i=0; i<(int)items_to_prepend.size(); ++i) {
      render_definition(items_to_prepend[i].first.c_str(), items_to_prepend[i].second.c_str());
      /*
      fprintf(macdic_xml_fp, "\n\t"); // DEBUG
      //fprintf(macdic_xml_fp, "<span class=\"wordclass\">%s</span><br />", items_to_prepend[i].first.c_str());
      //fprintf(macdic_xml_fp, "%s<br />", items_to_prepend[i].second.c_str());
      fprintf(macdic_xml_fp,
              "<span class=\"wordclass\">%s</span>",
              items_to_prepend[i].first.c_str());
      fprintf(macdic_xml_fp,
              "%s", items_to_prepend[i].second.c_str());
      */
    }
    //fprintf(macdic_xml_fp, "<br />\n");
  }

  if (level > 0) {
    fprintf(macdic_xml_fp, "<span class=\"wordclass\">レベル</span>%d<br />\n", level);
  }

  if (trap_mode_) printf("#   rendering...\n");

  // cout << "I:" << items << endl;
  for (int i=0; i<(int)items.size(); ++i) {
    render_definition(items[i].first.c_str(), items[i].second.c_str());
  }

  if (trap_mode_) printf("#   Z\n");

  fprintf(macdic_xml_fp, "</p>\n");
  fprintf(macdic_xml_fp, "</d:entry>\n");
  fflush(macdic_xml_fp);

  free(escaped_entry);
}

std::string idstr(const char *cstr) {
  size_t len = strlen(cstr);
  std::stringstream ss;

  unsigned char digest[16];
  //MD5((const unsigned char *)cstr, len, digest);
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, (const unsigned char *)cstr, len);
  MD5_Final(digest, &ctx);
  /*
  for (int i = 0; i < len; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)cstr[i];
  }
  */
  ss << '_';
  ss << std::hex << std::setw(2) << std::setfill('0');
  ss << current_dict_id_ << '.';
  for (int i = 0; i < 16; ++i) {
    ss << (int)digest[i];
  }
  return ss.str();
}

char *escape(const char *str) {
  int len = strlen(str);
  int cnt[256];
  for (int i=0; i<256; ++i) cnt[i] = 0;

  int bufsize = len;
  for (int i=0; i<len; ++i) {
    switch (str[i]) {
      case '&': // -> "&amp;" (+4)
        bufsize += 4; break;
      case '"': // -> "&quot;" (+5)
        bufsize += 5; break;
      case '<': // -> "&lt;" (+3)
        bufsize += 3; break;
      case '>': // -> "&gt;" (+3)
        bufsize += 3; break;
      default:
        break;
    }
  }
  char *buf = (char *)malloc(bufsize + 1);
  for (int i=0, j=0; i<len; ++i) {
    switch (str[i]) {
      case '&':
        memcpy(buf+j, "&amp;", 5);
        j += 5; break;
      case '"':
        memcpy(buf+j, "&quot;", 6);
        j += 6; break;
      case '<':
        memcpy(buf+j, "&lt;", 4);
        j += 4; break;
      case '>':
        memcpy(buf+j, "&gt;", 4);
        j += 4; break;
      default:
        buf[j++] = str[i];
        break;
    }
  }
  buf[bufsize] = 0;
  return buf;
}

void output(const char *begin, const char *end) {
  int size = (end == NULL)? strlen(begin) : (end - begin);
  if (trap_mode_) {
    printf("#   output(%d): ", size);
    fwrite(begin, 1, size, stdout);
    printf("\n");
  }
  fwrite(begin, 1, size, macdic_xml_fp);
}

void parse_output(const char *begin, const char *end) {
  //int size = (end == NULL)? strlen(begin) : (end - begin);
  if (!end) end = begin + strlen(begin);
  if (trap_mode_) {
    printf("#   parse_output: size=%d, ", (int)(end - begin));
    int len = end ? (end - begin) : strlen(begin);
    fwrite(begin, 1, len, stdout);
    printf("\n");
  }

  const char *ql = strnstr(begin, "&lt;→", end-begin);
  if (ql) {
    const char *qlE = strnstr(ql+7, "&gt;", end-(ql+7));
    // ql+7 ... qlE
    int word_len = (int)(qlE - (ql+7));
    char *word_buf = (char *)malloc(word_len + 1);
    memcpy(word_buf, ql+7, word_len);
    word_buf[word_len] = 0;
    fprintf(macdic_xml_fp,
            "&lt;→<a href=\"x-dictionary:r:%s\">%s</a>&gt;",
            idstr(word_buf).c_str(),
            word_buf);
    free(word_buf);

    begin = qlE + 4;
  }

  const char *qf = strnstr(begin, "◆file:", end-begin);
  //const char *qf = strstr(begin, "◆file");
  // printf("qf: %p\n", qf);
  if (trap_mode_) printf("#   P 1\n");

  if (!qf) {
    output(begin, end);
  } else {
    output(begin, qf);

    qf += 8; // 8 = strlen("◆file:");
    //const char *qE = strnstr(qf, ".TXT", end-qf);
    const char *qE = strnstr(qf, ".TXT", 12);
    if (qE) qE += 4;
    else {
      //qE = strnstr(qf, ".HTM", end-qf);
      qE = strnstr(qf, ".HTM", 12);
      if (qE) qE += 4;
      else {
        qE = end;
        printf("** file:%s\n", qf);
      }
    }
 
    char fname_buf[100];
    memcpy(fname_buf, qf, qE-qf);
    fname_buf[qE-qf] = 0;
    if (trap_mode_) printf("#   F %s\n", fname_buf);
    fprintf(macdic_xml_fp,
            "◆<a href=\"x-dictionary:r:f.%s\">file:%s</a>",
            fname_buf, // id
            fname_buf);
    // output(qf, qE-qf);
    // fprintf(macdic_xml_fp, "</a>");
    if (qE < end) {
      output(qE, end);
    }
  }
  fprintf(macdic_xml_fp, "<br />\n");
  if (trap_mode_) printf("#   P z\n");
}

void render_definition(const char *wordclass, const char *desc) {
  // fprintf(macdic_xml_fp, "TRAP(%s)(%s)\n", wordclass, desc);
  if (trap_mode_) {
    printf("#   render_definition: %s | %s\n", wordclass, desc);
  }
  
  fprintf(macdic_xml_fp, "\n\t"); // DEBUG
  if (wordclass && strlen(wordclass) > 0) {
    fprintf(macdic_xml_fp, "<span class=\"wordclass\">%s</span>", wordclass);
    /// fprintf(macdic_xml_fp, "<br />\n");
  }

  if (trap_mode_) printf("#   R 1\n");

  bool example_mode = false;
  char *escaped = escape(desc);
  // fprintf(macdic_xml_fp, "%s", escaped);
  const char *p = escaped, *pE = p + strlen(escaped);
  while (p < pE) {
    const char *q = strstr(p, "\r\n・");
    if (!q) break;

    if (example_mode) {
      fprintf(macdic_xml_fp, "\t<span class=\"example\">・");
      output(p, q);
      fprintf(macdic_xml_fp, "</span><br />\n");
    } else {
      parse_output(p, q);
      example_mode = true;
    }

    p = q + 5; // strlen("\r\n・");
  }

  if (trap_mode_) printf("#   R 2\n");

  if (example_mode) {
    // printf("・");
    fprintf(macdic_xml_fp, "\t<span class=\"example\">・%s</span><br />\n", p);
  } else {
    parse_output(p);
  }
  /// fprintf(macdic_xml_fp, "<br />\n");

  //  if (p) {
  //    fprintf(macdic_xml_fp, "[[・FOUND]]\n");
  //  }

  if (trap_mode_) printf("#   R 3\n");

  // render_content(content);
  // ^・.....$ -> 例文
  // "◆file:_TORTO.TXT" -> リンク化

  free(escaped);
}

