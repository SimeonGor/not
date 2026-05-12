#include "communication/comm_service.h"

void CommService::begin() {
  status_ = NETWORK_OFFLINE;
  bindSuccessPending_ = false;
  incomingMessagePending_ = false;
}

void CommService::loop() {
  // На КТ1 сеть отключена. Здесь позже появится неблокирующий reconnect Wi-Fi/MQTT.
}

NetworkStatus CommService::getStatus() const {
  return status_;
}

bool CommService::isOffline() const {
  return status_ == NETWORK_OFFLINE;
}

bool CommService::consumeBindSuccess() {
  if (!bindSuccessPending_) {
    return false;
  }

  bindSuccessPending_ = false;
  return true;
}

bool CommService::consumeIncomingMessage(char *buffer, const size_t bufferSize) {
  if (!incomingMessagePending_ || buffer == nullptr || bufferSize == 0) {
    return false;
  }

  buffer[0] = '\0';
  incomingMessagePending_ = false;
  return false;
}
