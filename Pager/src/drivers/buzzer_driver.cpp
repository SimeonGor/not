#include "drivers/buzzer_driver.h"

#include "config.h"

void BuzzerDriver::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  off_();
  phase_ = kPhaseIdle;
  phaseEndMs_ = 0;
  pulsesRemaining_ = 0;
}

void BuzzerDriver::off_() {
  digitalWrite(PIN_BUZZER, LOW);
}

void BuzzerDriver::startClickBeep() {
  phase_ = kPhaseOn;
  pulsesRemaining_ = 1;
  digitalWrite(PIN_BUZZER, HIGH);
  phaseEndMs_ = millis() + kOnMs;
}

void BuzzerDriver::startIncomingBeep() {
  phase_ = kPhaseOn;
  pulsesRemaining_ = 3;
  digitalWrite(PIN_BUZZER, HIGH);
  phaseEndMs_ = millis() + kOnMs;
}

void BuzzerDriver::update(const uint32_t nowMs) {
  if (phase_ == kPhaseIdle) {
    return;
  }

  if (static_cast<int32_t>(nowMs - phaseEndMs_) < 0) {
    return;
  }

  if (phase_ == kPhaseOn) {
    off_();
    pulsesRemaining_--;
    if (pulsesRemaining_ == 0) {
      phase_ = kPhaseIdle;
      return;
    }
    phase_ = kPhaseGap;
    phaseEndMs_ = nowMs + kGapMs;
    return;
  }

  phase_ = kPhaseOn;
  digitalWrite(PIN_BUZZER, HIGH);
  phaseEndMs_ = nowMs + kOnMs;
}
