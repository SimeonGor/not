#pragma once

#include <Arduino.h>

#include "config.h"

/** Локальная история rx-сообщений только в RAM (Phase 5A). */
class MessageHistoryStore {
 public:
  void begin();

  /** Добавить сообщение: [0] — самое новое, сдвиг вниз, count ≤ HISTORY_SIZE. */
  void addMessageToHistory(const String &msg);

  int count() const { return historyCount_; }
  int selectedHistoryIndex() const { return selectedHistoryIndex_; }
  void setSelectedHistoryIndex(int idx);

  /** Сообщение по индексу 0..count-1 (0 = самое новое). */
  String messageAt(int index) const;

  static String makeMessagePreview(const String &msg, int maxLen);

 private:
  String messageHistory_[HISTORY_SIZE];
  int historyCount_{0};
  int selectedHistoryIndex_{0};
};
