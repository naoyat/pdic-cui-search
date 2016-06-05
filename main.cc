// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./Shell.h"

int main(int argc, char **argv) {
  Shell shell;

  if (argc >= 2) {
      int size = 0;
      for (int i = 1; i < argc; ++i) {
          size += strlen(argv[i]) + 1;
      }
      char *line = new char[size], *p = line;
      line[0] = '\0';
      for (int i = 1; i < argc; ++i) {
          int n = strlen(argv[i]);
          strncpy(p, argv[i], n);
          p += n;
          *p++ = (i == argc - 1) ? '\0' : ' ';
      }
      // printf("(%s)\n", line);
      shell.run(line);

      delete[] line;
  } else {
      shell.run();
  }

  return 0;
}
