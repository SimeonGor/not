#include "drivers/buzzer_driver.h"

#include "config.h"

void BuzzerDriver::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  off_();
  phase_ = kPhaseIdle;
  phaseEndMs_ = 0;
  pulsesRemaining_ = 0;
  successTone_ = false;
}

void BuzzerDriver::off_() {
  digitalWrite(PIN_BUZZER, LOW);
}

void BuzzerDriver::startClickBeep() {
  // Не перезапускать импульс, пока предыдущий не завершён: иначе каждый вызов
  // сбрасывает phaseEndMs_ и update() никогда не гасит пин — получается «залипший» HIGH.
  if (phase_ != kPhaseIdle) {
    return;
  }
  successTone_ = false;
  phase_ = kPhaseOn;
  pulsesRemaining_ = 1;
  digitalWrite(PIN_BUZZER, HIGH);
  phaseEndMs_ = millis() + kOnMs;
}

void BuzzerDriver::startIncomingBeep() {
  successTone_ = false;
  phase_ = kPhaseOn;
  pulsesRemaining_ = 3;
  digitalWrite(PIN_BUZZER, HIGH);
  phaseEndMs_ = millis() + kOnMs;
}

void BuzzerDriver::startSuccessBeep() {
  successTone_ = true;
  phase_ = kPhaseOn;
  pulsesRemaining_ = 2;
  digitalWrite(PIN_BUZZER, HIGH);
  phaseEndMs_ = millis() + kSuccessOnMs;
}

void BuzzerDriver::update(const uint32_t nowMs) {
  if (phase_ == kPhaseIdle) {
    return;
  }

  if (static_cast<int32_t>(nowMs - phaseEndMs_) < 0) {
    return;
  }

  const uint16_t onMs = successTone_ ? kSuccessOnMs : kOnMs;
  const uint16_t gapMs = successTone_ ? kSuccessGapMs : kGapMs;

  if (phase_ == kPhaseOn) {
    off_();
    pulsesRemaining_--;
    if (pulsesRemaining_ == 0) {
      phase_ = kPhaseIdle;
      successTone_ = false;
      return;
    }
    phase_ = kPhaseGap;
    phaseEndMs_ = nowMs + gapMs;
    return;
  }

  phase_ = kPhaseOn;
  digitalWrite(PIN_BUZZER, HIGH);
  phaseEndMs_ = nowMs + onMs;
}
