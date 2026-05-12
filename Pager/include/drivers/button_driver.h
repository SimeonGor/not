#pragma once

#include "pager_types.h"

class ButtonDriver {
 public:
  void begin();
  ButtonEvent readInputs();

 private:
  struct ButtonState {
    uint8_t pin;
    bool stablePressed;
    bool lastReading;
    uint32_t lastChangeMs;
  };

  ButtonState buttons_[3];
  ButtonEvent sampleButton_(ButtonState &button, uint32_t nowMs);
};
