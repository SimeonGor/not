#include "services/message_history.h"

void MessageHistoryStore::begin() {
  for (int i = 0; i < HISTORY_SIZE; ++i) {
    messageHistory_[i] = "";
  }
  historyCount_ = 0;
  selectedHistoryIndex_ = 0;
}

String MessageHistoryStore::makeMessagePreview(const String &msg, const int maxLen) {
  if (maxLen <= 3) {
    return msg;
  }
  if (msg.length() <= static_cast<unsigned>(maxLen)) {
    return msg;
  }
  return msg.substring(0, maxLen - 3) + "...";
}

void MessageHistoryStore::addMessageToHistory(const String &msg) {
  String m = msg;
  if (m.length() > static_cast<int>(HISTORY_MESSAGE_MAX_LEN)) {
    m = m.substring(0, HISTORY_MESSAGE_MAX_LEN);
  }

  for (int i = HISTORY_SIZE - 1; i > 0; --i) {
    messageHistory_[i] = messageHistory_[i - 1];
  }
  messageHistory_[0] = m;

  if (historyCount_ < HISTORY_SIZE) {
    historyCount_++;
  }

  Serial.print(F("[HISTORY] Added message, count="));
  Serial.println(historyCount_);
}

void MessageHistoryStore::setSelectedHistoryIndex(const int idx) {
  if (historyCount_ <= 0) {
    selectedHistoryIndex_ = 0;
    return;
  }
  if (idx < 0) {
    selectedHistoryIndex_ = 0;
  } else if (idx >= historyCount_) {
    selectedHistoryIndex_ = historyCount_ - 1;
  } else {
    selectedHistoryIndex_ = idx;
  }
}

String MessageHistoryStore::messageAt(const int index) const {
  if (index < 0 || index >= historyCount_) {
    return String();
  }
  return messageHistory_[index];
}
