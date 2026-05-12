#pragma once

#include <Arduino.h>

class BuzzerDriver {
 public:
  void begin();
  void update(uint32_t nowMs);
  void startClickBeep();
  void startIncomingBeep();

 private:
  enum BuzzerPhase : uint8_t {
    kPhaseIdle = 0,
    kPhaseOn,
    kPhaseGap
  };

  void off_();

  BuzzerPhase phase_;
  uint32_t phaseEndMs_;
  uint8_t pulsesRemaining_;
  static const uint16_t kOnMs = 60;
  static const uint16_t kGapMs = 80;
};
