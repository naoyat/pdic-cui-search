#include "timeutil.h"

#include <ctime>
#include <sys/time.h>

timeval _timeval_start;
clock_t _clock_start;

void time_reset() {
  _clock_start = clock();
  gettimeofday(&_timeval_start, NULL);
}

std::pair<int,int> time_usec() {
  int timeval_usec, clock_usec;
  timeval timeval_end;
  gettimeofday(&timeval_end, NULL);
  clock_t clock_end = clock();
  timeval_usec = (timeval_end.tv_sec - _timeval_start.tv_sec)*1000000 + (timeval_end.tv_usec - _timeval_start.tv_usec);
  if (CLOCKS_PER_SEC == 1000000)
    clock_usec = clock_end - _clock_start;
  else
    clock_usec = (int)((long long)(clock_end - _clock_start) * 1000000 / CLOCKS_PER_SEC);

  return std::make_pair(timeval_usec, clock_usec);
}