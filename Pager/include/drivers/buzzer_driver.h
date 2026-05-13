#pragma once

#include <Arduino.h>

class BuzzerDriver {
 public:
  void begin();
  void update(uint32_t nowMs);
  void startClickBeep();
  void startIncomingBeep();
  void startSuccessBeep();

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
  bool successTone_{false};
  static const uint16_t kOnMs = 60;
  static const uint16_t kGapMs = 80;
  static const uint16_t kSuccessOnMs = 45;
  static const uint16_t kSuccessGapMs = 55;
};
