#include "drivers/button_driver.h"

#include "config.h"

void ButtonDriver::begin() {
  buttons_[0] = {PIN_BTN_UP, false, true, 0};
  buttons_[1] = {PIN_BTN_DOWN, false, true, 0};
  buttons_[2] = {PIN_BTN_OK, false, true, 0};

  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  pinMode(PIN_BTN_OK, INPUT_PULLUP);
}

ButtonEvent ButtonDriver::sampleButton_(ButtonState &button, const uint32_t nowMs) {
  const bool rawPressed = (digitalRead(button.pin) == LOW);

  if (rawPressed != button.lastReading) {
    button.lastChangeMs = nowMs;
    button.lastReading = rawPressed;
  }

  if ((nowMs - button.lastChangeMs) < DEBOUNCE_MS) {
    return BTN_EVENT_NONE;
  }

  if (!button.stablePressed && rawPressed) {
    button.stablePressed = true;
    return BTN_EVENT_NONE;
  }

  if (button.stablePressed && !rawPressed) {
    button.stablePressed = false;
    if (button.pin == PIN_BTN_UP) {
      return BTN_EVENT_UP;
    }
    if (button.pin == PIN_BTN_DOWN) {
      return BTN_EVENT_DOWN;
    }
    return BTN_EVENT_OK;
  }

  return BTN_EVENT_NONE;
}

ButtonEvent ButtonDriver::readInputs() {
  const uint32_t nowMs = millis();
  ButtonEvent event = BTN_EVENT_NONE;

  for (uint8_t i = 0; i < 3; i++) {
    const ButtonEvent sample = sampleButton_(buttons_[i], nowMs);
    if (sample != BTN_EVENT_NONE) {
      event = sample;
    }
  }

  return event;
}
