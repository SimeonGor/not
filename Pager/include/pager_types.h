#pragma once

#include <Arduino.h>

#include "config.h"

enum SystemState {
  STATE_PAIRING,
  STATE_IDLE,
  STATE_READING
};

enum ButtonEvent {
  BTN_EVENT_NONE = 0,
  BTN_EVENT_UP,
  BTN_EVENT_DOWN,
  BTN_EVENT_OK
};

enum NetworkStatus {
  NETWORK_OFFLINE,
  NETWORK_CONNECTING,
  NETWORK_ONLINE
};

struct TextLayout {
  char lines[MAX_WRAPPED_LINES][LINE_CHAR_CAPACITY + 1];
  uint8_t lineCount;
  int16_t maxScrollOffset;
};

struct PagerViewModel {
  SystemState state;
  NetworkStatus networkStatus;
  uint16_t pairingPin;
  int16_t scrollOffset;
  const TextLayout *messageLayout;
  char deviceId[13];
  bool wifiConnected;
  bool mqttConnected;
};
