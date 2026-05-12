#pragma once

#include "pager_types.h"

// Слой коммуникации: Wi-Fi/MQTT. На КТ1 работает в offline-режиме.
class CommService {
 public:
  void begin();
  void loop();

  NetworkStatus getStatus() const;
  bool isOffline() const;
  bool consumeBindSuccess();
  bool consumeIncomingMessage(char *buffer, size_t bufferSize);

 private:
  NetworkStatus status_;
  bool bindSuccessPending_;
  bool incomingMessagePending_;
};
