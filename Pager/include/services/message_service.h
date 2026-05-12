#pragma once

#include <stddef.h>

#include "pager_types.h"

class MessageService {
 public:
  void begin();
  const TextLayout &getLayout() const;

  void setIncomingText(const char *text, size_t maxLen);
  void loadMockForLocalTest();
  void clearToEmpty();

 private:
  static const char MOCK_MESSAGE[] PROGMEM;

  TextLayout layout_;
  char rawBuffer_[RX_MESSAGE_MAX_LEN + 1];
};
