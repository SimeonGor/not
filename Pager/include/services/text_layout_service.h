#pragma once

#include "pager_types.h"

class TextLayoutService {
 public:
  TextLayout wrapProgmemText(const char *textFromProgmem);
  TextLayout wrapRamText(const char *textFromRam);
  int16_t clampScrollOffset(int16_t scrollOffset, int16_t maxScrollOffset) const;
};
