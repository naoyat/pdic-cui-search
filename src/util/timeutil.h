// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_TIMEUTIL_H_
#define UTIL_TIMEUTIL_H_

#include <utility>

void time_reset();
std::pair<int, int> time_usec();

#endif  // UTIL_TIMEUTIL_H_
