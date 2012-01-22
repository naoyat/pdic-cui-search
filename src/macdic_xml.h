#ifndef PDICCUISEARCH_MACDICXML_H_
#define PDICCUISEARCH_MACDICXML_H_

#include <string>
#include <cstdio>

class PDICDatafield;

int macdic_xml_open(std::string path);
void cb_macdic_xml(PDICDatafield *datafield);
void macdic_xml_close();

#endif // PDICCUISEARCH_MACDICXML_H_
