#include "drivers/buzzer_driver.h"

#include "config.h"

void BuzzerDriver::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  off_();
}

void BuzzerDriver::off_() {
  digitalWrite(PIN_BUZZER, LOW);
}

void BuzzerDriver::beep(const uint16_t durationMs) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  off_();
}

void BuzzerDriver::beepIncomingMessage() {
  const uint8_t beepCount = 2;
  for (uint8_t i = 0; i < beepCount; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(BEEP_MESSAGE_ON_MS);
    off_();
    if (i + 1 < beepCount) {
      delay(BEEP_MESSAGE_OFF_MS);
    }
  }
}
