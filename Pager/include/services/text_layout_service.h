#pragma once

#include "pager_types.h"

class TextLayoutService {
 public:
  TextLayout wrapProgmemText(const char *textFromProgmem);
  TextLayout wrapRamText(const char *textFromRam);
  int16_t clampScrollLine(int16_t scrollLine, int16_t maxScrollLines) const;
};
