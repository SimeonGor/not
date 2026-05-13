#pragma once

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "pager_types.h"

enum class MqttSysCommand : uint8_t { None = 0, BindSuccess, Unbound };

class CommService {
 public:
  CommService();

  void begin();
  void loop();

  NetworkStatus getStatus() const;
  bool isWifiConnected() const;
  bool isMqttConnected();
  void getDeviceId(char out[13]) const;

  bool takeRxPayload(char *out, size_t outSize);
  MqttSysCommand takePendingSysCommand();

  /** Разрешить приём payload с `pager/.../rx` только при привязанном устройстве. */
  void setDeviceBoundForRx(bool bound);

  void onMqttMessage(char *topic, byte *payload, unsigned int length);

 private:
  void updateWifi_(uint32_t nowMs);
  void updateMqtt_(uint32_t nowMs);
  void tryMqttConnect_();
  void refreshDeviceId_();

  WiFiClient wifiClient_;
  PubSubClient mqttClient_;

  char deviceId_[13];
  char topicRx_[40];
  char topicSys_[40];

  char rxBuffer_[RX_MESSAGE_MAX_LEN + 1];
  volatile bool rxPending_{false};

  char sysBuffer_[32];
  volatile bool sysPending_{false};

  bool deviceBoundForRx_{false};

  uint32_t lastWifiAttemptMs_;
  uint32_t lastMqttAttemptMs_;
  bool wifiWasConnected_;
  bool mqttWasConnected_;
};
