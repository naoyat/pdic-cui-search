#include "macdic_xml.h"

#include <stdio.h>

#include "Dict.h"
#include "PDICDatafield.h"


FILE *macdic_xml_fp = NULL;
int macdic_xml_entry_id = 0;

int macdic_xml_open(std::string path)
{
  macdic_xml_fp = fopen(path.c_str(), "w");
  if (!macdic_xml_fp) return -1;

  macdic_xml_entry_id = 0;

  // header
  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", macdic_xml_fp);
  fputs("<d:dictionary xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:d=\"http://www.apple.com/DTDs/DictionaryService-1.0.rng\">\n", macdic_xml_fp);

  return 0;
}

void macdic_xml_close()
{
  // footer
  fputs("</d:dictionary>\n", macdic_xml_fp);

  fclose(macdic_xml_fp); macdic_xml_fp = NULL;
}

void cb_macdic_xml(PDICDatafield *datafield)
{
  byte *entry_word = datafield->in_utf8(F_ENTRY);
  byte *jword = datafield->in_utf8(F_JWORD);

  fprintf(macdic_xml_fp, "\t<d:entry id=\"%d\" d:title=\"%s\">\n",
          ++macdic_xml_entry_id,
          entry_word);
  fprintf(macdic_xml_fp, "\t<d:index d:value=\"%s\" />\n", entry_word);
  /*
  // word_conj[x] から value を抜いて
  for (elem in word_conj[x]) {
    // ハッシュに格納してある変化形でも引けるようにする
    printf("\t<d:index d:value=\"%s\" " "d:title=\"%s (%s)\" />\n", elem, elem, entry_word);
  }
  */
  fprintf(macdic_xml_fp, "\t\t<h1>%s</h1>\n", entry_word);
    //word_definition[x] = escapeHTML(word_definition[x]); // HTMLのタグをエスケープ
    
  fprintf(macdic_xml_fp, "\t\t<p>%s</p>\n", jword);
  fprintf(macdic_xml_fp, "\t</d:index>\n");
}
