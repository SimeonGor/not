#pragma once

#include <Arduino.h>

#include "config.h"

enum SystemState {
  STATE_BOOT,
  STATE_CONNECTING,
  STATE_PAIRING,
  STATE_IDLE,
  STATE_READING,
  STATE_HISTORY_LIST,
  STATE_HISTORY_READING,
  STATE_ERROR
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
  /** Текущий PIN (до 6 цифр + '\0'); пусто если нет активного PIN. */
  char pairingPin[PAIRING_PIN_MAX_LEN + 1];
  int pairingRemainingSeconds;
  bool pairingRefreshing;
  bool pairingBackendError;
  bool backendReachable;
  bool isBound;
  /** Прокрутка текста в целых строках (высота строки = LINE_HEIGHT px). */
  int16_t scrollLine;
  const TextLayout *messageLayout;
  char deviceId[13];
  /** Последние символы deviceId для IDLE (например 6). */
  char deviceIdTail[7];
  bool wifiConnected;
  bool mqttConnected;
  /** Кратко показать «привязано» после BIND_SUCCESS. */
  bool bindSuccessFlash;
  bool factoryResetErrorFlash;

  /** Phase 5A: локальная история (RAM). */
  uint8_t histCount;
  uint8_t histSelected;
  bool historyReadingMode;
  uint8_t histReadX;
  uint8_t histReadY;
  char histListRow0[HISTORY_LIST_ROW_BUF];
  char histListRow1[HISTORY_LIST_ROW_BUF];
  char histListRow2[HISTORY_LIST_ROW_BUF];
};
