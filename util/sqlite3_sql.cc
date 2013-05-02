// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "util/sqlite3_sql.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <re2/re2.h>
#include <re2/stringpiece.h>

#include <iomanip>  // setfill
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include "pdic/Dict.h"
#include "pdic/PDICDatafield.h"
#include "util/stlutil.h"

extern int dump_remain_count_;

FILE *sqlite3_sql_fp = NULL;
int sqlite3_sql_entry_id_ = 0;

std::map<std::string, int> revs;
std::map<int, std::string> aliases;

// static
char *sql_escape(const char *str);
void sql_parse_output(const char *begin, const char *end = NULL);
void sql_render_definition(int id, const char *wordclass, const char *desc);

int sqlite3_sql_open(std::string path) {
  printf("[%s]", path.c_str());

  sqlite3_sql_fp = fopen(path.c_str(), "w");
  if (!sqlite3_sql_fp) return -1;

  revs.clear();
  aliases.clear();

  // sqlite3_sql_entry_id_ = 0;
  return 0;
}

void sqlite3_sql_close() {
  traverse(aliases, it) {
#ifdef SQL_TABSEP
    fprintf(sqlite3_sql_fp, "aliases\t%d\t%s\t", it->first, it->second.c_str());
    if (found(revs, it->second)) {
      fprintf(sqlite3_sql_fp, "%d\n", revs[it->second]);
    } else {
      fprintf(sqlite3_sql_fp, "\n");
    }
#else
    fprintf(sqlite3_sql_fp,
            "INSERT INTO aliases "
            "(from_id, to_phrase, to_id) "
            "VALUES (%d, '%s', ",
            it->first,
            it->second.c_str());
    if (found(revs, it->second)) {
      fprintf(sqlite3_sql_fp, " %d);\n", revs[it->second]);
    } else {
      fprintf(sqlite3_sql_fp, " NULL);\n");
    }
#endif
  }

  fclose(sqlite3_sql_fp);
  sqlite3_sql_fp = NULL;

  printf(" sqlite3_sql_entry_id = %d\n", sqlite3_sql_entry_id_);
  printf("   revs: %d, aliases: %d\n",
         static_cast<int>(revs.size()),
         static_cast<int>(aliases.size()));
}

void cb_sqlite3_sql(PDICDatafield *datafield) {
  byte *entry    = datafield->in_utf8(F_ENTRY);
  byte *jword    = datafield->in_utf8(F_JWORD);
  // byte *example  = datafield->in_utf8(F_EXAMPLE); // unused
  byte *pron     = datafield->in_utf8(F_PRON);
  int   level    = datafield->entry_word_level;

  if (!entry) return;
  if (!jword) return;

  // if (!(entry[0] == 'a' || entry[0] == 'A' ||
  //    strncmp("あ", (const char *)entry, 3)==0)) return;
  // if (!strcmp("走る", (const char *)entry)==0) return;

  sqlite3_sql_entry_id_++;
  --dump_remain_count_;

  int jl = strlen(reinterpret_cast<char *>(jword));

  std::vector<std::pair<int, int> > offsets;

  const char *p0 = reinterpret_cast<const char *>(jword), *pE = p0 + jl;

  for (const char *p = p0; p < pE; ) {
    // printf("..%d/%d", p-p0, jl);
    const char *q1 = strstr(p, "【");
    if (!q1) break;
    if (q1 >= p0+3) {
      // "【" の直前の文字が ◆ や〔 ならスキップ
      if (strncmp(q1-3, "◆", 3) == 0 || strncmp(q1-3, "〔", 3) == 0) {
        p = q1 + 3;
        continue;
      }
    }
    const char *q2 = strstr(q1+3, "】");
    if (!q2) break;

    offsets.push_back(std::make_pair(q1-p0, (q2+3)-p0));
    p = q2 + 3;
  }

  int n = static_cast<int>(offsets.size());

  std::vector<std::pair<std::string, std::string> > items;

  std::set<std::string> conj_forms;

  if (n > 0) {
    if (offsets[0].first > 0) {
      std::string s2(reinterpret_cast<const char *>(jword), offsets[0].first);
      items.push_back(std::make_pair("", s2));
    }

    // 【】あり
    offsets.push_back(std::make_pair(jl, 0));

    for (int i = 0; i < n; ++i) {
      int j1 = offsets[i].first, j2 = offsets[i].second;
      int j1next = offsets[i+1].first;

      int s1_from = j1 + 3, s1_len = j2 - j1 - 6;
      int s2_from = j2,     s2_len = j1next - j2;

      if (p0[s2_from + s2_len - 2] == 0x0d) s2_len -= 2;
      // if (p0[s2_from + s2_len - 1] == 0x0a) s2_len -= 1;
      // if (p0[s2_from + s2_len - 1] == '\n') --s2_len;

      // 末尾の「、」を切り捨てる
      if (strncmp("、", p0 + s2_from + s2_len - 3, 3) == 0) s2_len -= 3;

      std::string s1(p0 + s1_from, s1_len);
      std::string s2(p0 + s2_from, s2_len);

      /*
      if (s1 == "＠" || s1 == "変化" || s1 == "文節") {
        prepend_mode = true;
        if (s1 == "＠") s1 = string("カナ");
      }
      */

      items.push_back(std::make_pair(s1, s2));

      if (s1 == "変化") {
        std::string s2a(s2.begin(), s2.end());
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
#ifdef SHOW_HENKA
        printf("<変化> \"%s\"\n", s2a.c_str());
#endif
        re2::StringPiece input(s2a);
        std::string var;

        // while (RE2::Consume(&input, "([A-Za-z][-a-z()]*)|", &var)) {
        while (RE2::Consume(&input, "([^|]*)\\|", &var)) {
          if (var == "") continue;
          std::string pre, paren, post;
          if (RE2::FullMatch(var, "([-A-Za-z]*)\\(([^)]*)\\)([-A-Za-z]*)",
                             &pre, &paren, &post)) {
#ifdef SHOW_HENKA
            printf("  - %s%s%s | %s%s\n",
                   pre.c_str(), paren.c_str(), post.c_str(),
                   pre.c_str(), post.c_str());
#endif
            conj_forms.insert(pre + paren + post);
            conj_forms.insert(pre + post);
          } else {
#ifdef SHOW_HENKA
            printf("  - %s\n", var.c_str());
#endif
            conj_forms.insert(var);
          }
        }
      }
    }
  } else {
    // 【】なし
    std::string s2(reinterpret_cast<const char *>(jword), jl);
    items.push_back(std::make_pair("", s2));
  }

  // entryをescapeしないと (" -> &quot; とか)

#ifdef SQL_TABSEP
  // "<d:entry id=\"%s\" d:title=\"%s\">",
  revs[reinterpret_cast<const char *>(entry)] = sqlite3_sql_entry_id_;

  fprintf(sqlite3_sql_fp, "entries\t%d\t%s\t%d\t",
          sqlite3_sql_entry_id_, entry, level);
  if (pron) {
    fprintf(sqlite3_sql_fp, "%s\n", pron);
  } else {
    fprintf(sqlite3_sql_fp, "\n");
  }
#else
  char *escaped_entry = sql_escape(reinterpret_cast<const char *>(entry));
  // "<d:entry id=\"%s\" d:title=\"%s\">",
  revs[escaped_entry] = sqlite3_sql_entry_id_;

  fprintf(sqlite3_sql_fp,
          "INSERT INTO entries "
          "(id, entry, level, pron) "
          "VALUES (%d, '%s', %d, );\n",
          sqlite3_sql_entry_id_,
          escaped_entry,
          level);
  if (pron) {
    fprintf(sqlite3_sql_fp, "'%s');\n", pron);
  } else {
    fprintf(sqlite3_sql_fp, "NULL);\n");
  }
#endif

  // ハッシュに格納してある変化形でも引けるようにする
  if (conj_forms.size() > 0) {
    for (typeof(conj_forms.begin()) it = conj_forms.begin();
         it != conj_forms.end();
         ++it) {
      const char *elem = it->c_str();
      if (strcmp(elem, reinterpret_cast<const char *>(entry)) == 0) continue;

#ifdef SQL_TABSEP
      // "<d:index d:value=\"%s\" " "d:title=\"%s (%s)\" />",
      fprintf(sqlite3_sql_fp, "surfaces\t%s\t%d\n",
              elem, sqlite3_sql_entry_id_);
#else
      char *escaped_elem = sql_escape(reinterpret_cast<const char *>(elem));
      // "<d:index d:value=\"%s\" " "d:title=\"%s (%s)\" />",
      fprintf(sqlite3_sql_fp,
              "INSERT INTO surfaces (surface, entry_id) VALUES ('%s', %d);\n",
              escaped_elem,
              sqlite3_sql_entry_id_);
      free(reinterpret_cast<void *>(escaped_elem));
#endif
    }
  }

  for (int i = 0, c = items.size(); i < c; ++i) {
    sql_render_definition(sqlite3_sql_entry_id_, items[i].first.c_str(),
                          items[i].second.c_str());
  }

  fflush(sqlite3_sql_fp);

#ifndef SQL_TABSEP
  free(escaped_entry);
#endif
}

char *sql_escape(const char *str) {
  int len = strlen(str);
  int cnt[256];
  for (int i = 0; i < 256; ++i) cnt[i] = 0;

  int bufsize = len;
  for (int i = 0; i < len; ++i) {
    switch (str[i]) {
      case '\'':  // -> "\'" (+1)
        bufsize += 1;
        break;
      default:
        break;
    }
  }
  char *buf = reinterpret_cast<char *>(malloc(bufsize + 1));
  for (int i = 0, j = 0; i < len; ++i) {
    switch (str[i]) {
      case '\'':
        memcpy(buf+j, "''", 2);  // Pascal style
        j += 2;
        break;
      default:
        buf[j++] = str[i];
        break;
    }
  }
  buf[bufsize] = 0;
  return buf;
}

void sql_parse_output(const char *begin, const char *end) {
  // int size = (end == NULL)? strlen(begin) : (end - begin);
  if (!end) end = begin + strlen(begin);

  std::string s(begin, end-begin);
  re2::StringPiece input(s);
  // RE2 re("(.*)<→([^>]*)>");
  RE2 re("<→ *([^>]*) *>");
  std::string /*pre,*/ other_entry;
  bool said = false;
  // while (re2::RE2::FindAndConsume(&input, re, &pre, &other_entry)) {
  while (re2::RE2::FindAndConsume(&input, re, &other_entry)) {
    if (!said) {
#ifdef SQL_TABSEP
      // fprintf(sqlite3_sql_fp, "%s\n", pre.c_str());
#else
      // fprintf(sqlite3_sql_fp, "'%s');\n", pre.c_str());
#endif
      // said = true;
    }
    aliases[sqlite3_sql_entry_id_] = other_entry;
  }

  if (!said) {
    std::string s(begin, end-begin);
#ifdef SQL_TABSEP
    fprintf(sqlite3_sql_fp, "%s\n", s.c_str());
#else
    fprintf(sqlite3_sql_fp, "'%s');\n", s.c_str());
#endif
    said = true;
  }
}

void sql_render_definition(int id, const char *wordclass, const char *desc) {
  // fprintf(sqlite3_sql_fp, "\n\t"); // DEBUG

  // fprintf(sqlite3_sql_fp, "<span class=\"wordclass\">%s</span>", wordclass);
  if (wordclass && strlen(wordclass) > 0) {
    std::string txt(wordclass);
    std::string a, b, c;
    if (RE2::FullMatch(txt, "(([1-9][0-9]*)-)?([^0-9]+)(-([1-9][0-9]*))?",
                       reinterpret_cast<void *>(NULL), &a, &b,
                       reinterpret_cast<void *>(NULL), &c)) {
      /*
      printf("// %s -> (%s) (%s) (%s)\n",
             wordclass,
             a.c_str(), b.c_str(), c.c_str());
      */
      int an = atoi(a.c_str()), cn = atoi(c.c_str());
#ifdef SQL_TABSEP
      fprintf(sqlite3_sql_fp, "definitions\t%d\t%d\t%s\t%d\t",
              id, an, b.c_str(), cn);
#else
      fprintf(sqlite3_sql_fp,
              "INSERT INTO definitions "
              "(entry_id, sub_id, wordclass, item_id, value) "
              "VALUES (%d, %d, '%s', %d, ",
              id, an, b.c_str(), cn);
#endif
    } else {
#ifdef SQL_TABSEP
      fprintf(sqlite3_sql_fp, "definitions\t%d\t\t%s\t\t", id, b.c_str());
#else
      fprintf(sqlite3_sql_fp,
              "INSERT INTO definitions "
              "(entry_id, wordclass, value) "
              "VALUES (%d, '%s', ",
              id, b.c_str());
#endif
    }
  } else {
    // printf("// %s\n", desc);
#ifdef SQL_TABSEP
    fprintf(sqlite3_sql_fp, "definitions\t%d\t\t\t\t", id);
#else
    fprintf(sqlite3_sql_fp,
            "INSERT INTO definitions (entry_id, value) VALUES (%d, ", id);
#endif
  }

  std::vector<std::string> examples;

  bool example_mode = false;
#ifdef SQL_TABSEP
  // fprintf(sqlite3_sql_fp, "%s", escaped);
  const char *p = desc, *pE = p + strlen(desc);
#else
  char *escaped = sql_escape(desc);
  // fprintf(sqlite3_sql_fp, "%s", escaped);
  const char *p = escaped, *pE = p + strlen(escaped);
#endif
  while (p < pE) {
    const char *q = strstr(p, "\r\n・");
    if (!q) break;

    std::string s(p, q-p);
    if (example_mode) {
      // fprintf(sqlite3_sql_fp, "\t<span class=\"example\">・");
      // std::string s(p, q-p);
      examples.push_back(s);
    } else {
      sql_parse_output(p, q);
      example_mode = true;
    }

    p = q + 5;  // strlen("\r\n・");
  }

  std::string s(p);
  if (example_mode) {
    // fprintf(sqlite3_sql_fp, "<span class=\"example\">・%s</span><br />", p);
    examples.push_back(s);
  } else {
    sql_parse_output(p);
  }

  for (int i = 0, c = examples.size(); i < c; ++i) {
#ifdef SQL_TABSEP
    fprintf(sqlite3_sql_fp, "definitions\t%d\t%d\t%s\t%d\t%s\n",
            id, 0, "example", 1+i, examples[i].c_str());
#else
    fprintf(sqlite3_sql_fp,
            "INSERT INTO definitions "
            "(entry_id, sub_id, wordclass, item_id, value) "
            "VALUES (%d, %d, '%s', %d, '%s');\n",
            id, 0, "example", 1+i, examples[i].c_str());
#endif
  }

#ifndef SQL_TABSEP
  free(escaped);
#endif
}
