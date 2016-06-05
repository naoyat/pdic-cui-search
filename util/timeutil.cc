// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./timeutil.h"

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include <utility>


timeval _timeval_start;
clock_t _clock_start;

void time_reset() {
  _clock_start = clock();
  gettimeofday(&_timeval_start, NULL);
}

std::pair<int, int> time_usec() {
  int timeval_usec, clock_usec;
  timeval timeval_end;
  gettimeofday(&timeval_end, NULL);
  clock_t clock_end = clock();
  timeval_usec = (timeval_end.tv_sec - _timeval_start.tv_sec)*1000000
      + (timeval_end.tv_usec - _timeval_start.tv_usec);
  if (CLOCKS_PER_SEC == 1000000) {
    clock_usec = clock_end - _clock_start;
  } else {
    clock_usec = static_cast<int>(
        static_cast<int64_t>(clock_end - _clock_start)
        * 1000000 / CLOCKS_PER_SEC);
  }
  return std::make_pair(timeval_usec, clock_usec);
}
