// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "util/macdic_xml.h"

#include <stdio.h>

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip> // setfill
// using namespace std;

#include "pdic/Dict.h"
#include "pdic/PDICDatafield.h"

extern int dump_remain_count;

FILE *macdic_xml_fp = NULL;
int macdic_xml_entry_id = 0;

std::map<std::string, std::set<std::string> > word_conj;
std::map<std::string, std::string> word_entry, word_definition, word_example, word_pron;

int macdic_xml_open(std::string path) {
  printf("[%s]", path.c_str());

  macdic_xml_fp = fopen(path.c_str(), "w");
  if (!macdic_xml_fp) return -1;

  word_conj.clear();
  word_entry.clear();
  word_definition.clear();
  word_example.clear();
  word_pron.clear();
  macdic_xml_entry_id = 0;
  return 0;
}

void macdic_xml_close() {
  printf(" macdic_xml_entry_id = %d\n", macdic_xml_entry_id);
#if 0
  printf(" (E:%d J:%d X:%d P:%d)\n",
         (int)word_entry.size(),
         (int)word_definition.size(),
         (int)word_example.size(),
         (int)word_pron.size());
  // header
  /*
  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", macdic_xml_fp);
  fputs("<d:dictionary xmlns=\"http://www.w3.org/1999/xhtml\" "
        "xmlns:d=\"http://www.apple.com/DTDs/DictionaryService-1.0.rng\">\n",
        macdic_xml_fp);
  */
  for (typeof(word_entry.begin()) it = word_entry.begin(); it != word_entry.end(); it++) {
    std::string id = it->first;
    std::string entry = it->second;
    std::string definition = word_definition[id];

    const char *d_id = id.c_str();
    const char *d_entry = entry.c_str();
    fprintf(macdic_xml_fp,
            "\n<d:entry id=\"%s\" d:title=\"%s\">\n",
            d_id, d_entry);

    fprintf(macdic_xml_fp, "\t<d:index d:value=\"%s\" />\n", d_entry);

    // fprintf(macdic_xml_fp, "\t<d:index d:value=\"%s\"", d_entry);
    // fprintf(macdic_xml_fp, " d:title=\"%s\" />", d_entry);

    for (typeof(word_conj[id].begin()) it = word_conj[id].begin();
         it != word_conj[id].end(); it++) {
      std::string elem = *it;
      // word_conj[x] から value を抜いて
      if (elem == entry) continue;
       // ハッシュに格納してある変化形でも引けるようにする
      const char *d_value = elem.c_str();
      fprintf(macdic_xml_fp,
              "\t<d:index d:value=\"%s\" " "d:title=\"%s (%s)\" />\n",
              d_value, d_value, d_entry);
    }

    fprintf(macdic_xml_fp, "\t<h1>%s</h1>\n", d_entry);
    if (word_pron.find(id) != word_pron.end()) {
      const char *d_pron = word_pron[id].c_str();
      fprintf(macdic_xml_fp, "\t<span class=\"syntax\"><span class=\"pr\">| %s |</span></span>\n",
              d_pron);
    }

    fprintf(macdic_xml_fp, "\t<p>"); //%s</p>\n", definition.c_str()); // 訳語部分

    fprintf(macdic_xml_fp, "%s", definition.c_str()); // 訳語部分
    // word_definition[x] = escapeHTML(word_definition[x]);

    fprintf(macdic_xml_fp, "</p>\n");
    fprintf(macdic_xml_fp, "</d:entry>\n");
  }
  // footer
  /*
  fputs("\n</d:dictionary>\n", macdic_xml_fp);
  */
#endif
  fclose(macdic_xml_fp);

  macdic_xml_fp = NULL;
}

std::string idstr(byte *cstr) {
  int len = strlen((char *)cstr);
  std::stringstream ss;
  for (int i = 0; i < len; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)cstr[i];
  }
  return ss.str();
}

void cb_macdic_xml(PDICDatafield *datafield) {
  // return;

  byte *entry    = datafield->in_utf8(F_ENTRY);
  byte *jword    = datafield->in_utf8(F_JWORD);
  byte *example  = datafield->in_utf8(F_EXAMPLE);
  byte *pron     = datafield->in_utf8(F_PRON);

  if (!pron) return; /// DEBUG
  // printf("cb_macdic_xml() %d %d %p\n", dump_remain_count, macdic_xml_entry_id, pron);

  macdic_xml_entry_id++;
  --dump_remain_count;

  std::string id = idstr(entry);

  /*
  std::cout << id << " ";
  printf("E:%s\n", entry_word);
  if (jword) printf("J:%s\n", jword);
  if (example) printf("S:%s\n", example);
  if (pron) printf("P:%s\n", pron);
  printf("\n");

  if (entry_word) { // ALL: 2039119
    std::string entry((const char *)entry_word);
    word_entry[id] = entry;
    word_conj[id].insert(entry);
  }
  if (jword) { // ALL: 2039119
    std::string definition((const char *)jword);
    word_definition[id] = definition;
  }

  //if (example) { // 0
  //  word_example[id] = std::string((const char *)example);
  //}
  if (pron) { // 27916
    std::string pronounciation = std::string((const char *)pron);
    word_pron[id] = pronounciation;
  }
  */
  /////
  //for (typeof(word_entry.begin()) it = word_entry.begin(); it != word_entry.end(); it++) {
  //std::string id = it->first;
  //std::string entry = it->second;
  //std::string definition = word_definition[id];

  const char *d_id = id.c_str();
  //const char *d_entry = entry.c_str();
  fprintf(macdic_xml_fp,
          "\n<d:entry id=\"%s\" d:title=\"%s\">\n",
          d_id, entry);

  fprintf(macdic_xml_fp, "\t<d:index d:value=\"%s\" />\n", entry);

    // fprintf(macdic_xml_fp, "\t<d:index d:value=\"%s\"", d_entry);
    // fprintf(macdic_xml_fp, " d:title=\"%s\" />", d_entry);
#ifdef WORD_CONJ
  for (typeof(word_conj[id].begin()) it = word_conj[id].begin();
       it != word_conj[id].end(); it++) {
    std::string elem = *it;
    // word_conj[x] から value を抜いて
    if (elem == entry) continue;
    // ハッシュに格納してある変化形でも引けるようにする
    const char *d_value = elem.c_str();
    fprintf(macdic_xml_fp,
            "\t<d:index d:value=\"%s\" " "d:title=\"%s (%s)\" />\n",
            d_value, d_value, d_entry);
  }
#endif
  fprintf(macdic_xml_fp, "\t<h1>%s</h1>\n", entry);
  if (pron) {
    fprintf(macdic_xml_fp, "\t<span class=\"syntax\"><span class=\"pr\">| %s |</span></span>\n",
            pron);
  }

  fprintf(macdic_xml_fp, "\t<p>"); //%s</p>\n", definition.c_str()); // 訳語部分

  fprintf(macdic_xml_fp, "%s", jword); // 訳語部分
  // word_definition[x] = escapeHTML(word_definition[x]);
  
  fprintf(macdic_xml_fp, "</p>\n");
  fprintf(macdic_xml_fp, "</d:entry>\n");
}
