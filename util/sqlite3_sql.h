// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_SQLITE3_SQL_H_
#define UTIL_SQLITE3_SQL_H_

#include <stdio.h>

#include <string>

class PDICDatafield;

int sqlite3_sql_open(std::string path);
void cb_sqlite3_sql(PDICDatafield* datafield);
void sqlite3_sql_close();

#endif  // UTIL_SQLITE3_SQL_H_
