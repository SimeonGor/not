#pragma once

#include "pager_types.h"

class MessageService {
 public:
  void begin();
  const TextLayout &getLayout() const;

 private:
  static const char MOCK_MESSAGE[] PROGMEM;

  TextLayout layout_;
};
