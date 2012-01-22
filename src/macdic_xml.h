// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef PDICCUISEARCH_MACDICXML_H_
#define PDICCUISEARCH_MACDICXML_H_

#include <stdio.h>

#include <string>


class PDICDatafield;


int macdic_xml_open(std::string path);
void cb_macdic_xml(PDICDatafield *datafield);
void macdic_xml_close();

#endif // PDICCUISEARCH_MACDICXML_H_
