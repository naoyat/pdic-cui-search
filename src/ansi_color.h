#ifndef ANSI_COLOR_H
#define ANSI_COLOR_H

#include <iostream>
#include <re2/re2.h>

#include "types.h"

void puts_emphasized(byte *str, const RE2& re);

#endif
