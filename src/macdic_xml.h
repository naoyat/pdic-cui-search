#ifndef MACDIC_XML_H
#define MACDIC_XML_H

#include <string>
#include <cstdio>

class PDICDatafield;

int macdic_xml_open(std::string path);
void cb_macdic_xml(PDICDatafield *datafield);
void macdic_xml_close();

#endif
