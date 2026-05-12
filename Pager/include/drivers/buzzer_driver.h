#pragma once

#include <Arduino.h>

class BuzzerDriver {
 public:
  void begin();
  void beep(uint16_t durationMs);
  void beepIncomingMessage();

 private:
  void off_();
};
