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
  /** Максимальный индекс прокрутки в строках (0 = верх текста). */
  int16_t maxScrollLines;
};

struct PagerViewModel {
  SystemState state;
  NetworkStatus networkStatus;
  uint16_t pairingPin;
  /** Прокрутка текста в целых строках (высота строки = LINE_HEIGHT px). */
  int16_t scrollLine;
  const TextLayout *messageLayout;
  char deviceId[13];
  bool wifiConnected;
  bool mqttConnected;
};
