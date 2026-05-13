#include "communication/comm_service.h"

#include <cstring>

#include "config.h"

namespace {
CommService *gComm = nullptr;

void mqttStaticCallback(char *topic, byte *payload, unsigned int length) {
  if (gComm != nullptr) {
    gComm->onMqttMessage(topic, payload, length);
  }
}

void trimInPlace(char *s) {
  if (s == nullptr || s[0] == '\0') {
    return;
  }
  char *end = s + strlen(s);
  while (end > s && (end[-1] == ' ' || end[-1] == '\r' || end[-1] == '\n' || end[-1] == '\t')) {
    *--end = '\0';
  }
  char *start = s;
  while (*start == ' ' || *start == '\r' || *start == '\n' || *start == '\t') {
    ++start;
  }
  if (start != s) {
    memmove(s, start, strlen(start) + 1);
  }
}
}  // namespace

CommService::CommService() : mqttClient_(wifiClient_) {}

void CommService::begin() {
  gComm = this;
  rxBuffer_[0] = '\0';
  rxPending_ = false;
  sysBuffer_[0] = '\0';
  sysPending_ = false;
  deviceBoundForRx_ = false;
  lastWifiAttemptMs_ = 0;
  lastMqttAttemptMs_ = 0;
  wifiWasConnected_ = false;
  mqttWasConnected_ = false;

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  refreshDeviceId_();
  snprintf(topicRx_, sizeof(topicRx_), "pager/%s/rx", deviceId_);
  snprintf(topicSys_, sizeof(topicSys_), "pager/%s/sys", deviceId_);

  mqttClient_.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient_.setBufferSize(512);
  mqttClient_.setCallback(mqttStaticCallback);

  Serial.println(F("[Comm] begin"));
  Serial.print(F("[Comm] deviceId="));
  Serial.println(deviceId_);
  Serial.print(F("[Comm] topicRx="));
  Serial.println(topicRx_);
  Serial.print(F("[Comm] topicSys="));
  Serial.println(topicSys_);
}

void CommService::setDeviceBoundForRx(const bool bound) {
  deviceBoundForRx_ = bound;
}

void CommService::refreshDeviceId_() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toUpperCase();
  mac.toCharArray(deviceId_, sizeof(deviceId_));
}

void CommService::loop() {
  const uint32_t nowMs = millis();
  updateWifi_(nowMs);
  updateMqtt_(nowMs);

  if (mqttClient_.connected()) {
    mqttClient_.loop();
  }
}

void CommService::updateWifi_(const uint32_t nowMs) {
  const wl_status_t st = WiFi.status();

  if (st == WL_CONNECTED) {
    if (!wifiWasConnected_) {
      wifiWasConnected_ = true;
      Serial.print(F("[Comm] WiFi connected, IP="));
      Serial.println(WiFi.localIP());
      lastMqttAttemptMs_ = 0;
    }
    return;
  }

  if (wifiWasConnected_) {
    wifiWasConnected_ = false;
    Serial.println(F("[Comm] WiFi lost"));
    mqttClient_.disconnect();
  }

  if (lastWifiAttemptMs_ == 0 ||
      (nowMs - lastWifiAttemptMs_) >= NETWORK_RETRY_INTERVAL_MS) {
    Serial.println(F("[Comm] WiFi.begin()"));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiAttemptMs_ = nowMs;
  }
}

void CommService::tryMqttConnect_() {
  const String clientId = String(F("pager-")) + deviceId_;
  Serial.print(F("[Comm] MQTT connect as "));
  Serial.println(clientId);

  if (mqttClient_.connect(clientId.c_str())) {
    if (mqttClient_.subscribe(topicRx_)) {
      Serial.print(F("[Comm] MQTT subscribed "));
      Serial.println(topicRx_);
    } else {
      Serial.println(F("[Comm] MQTT subscribe rx failed"));
    }
    if (mqttClient_.subscribe(topicSys_)) {
      Serial.print(F("[Comm] MQTT subscribed "));
      Serial.println(topicSys_);
    } else {
      Serial.println(F("[Comm] MQTT subscribe sys failed"));
    }
  } else {
    Serial.print(F("[Comm] MQTT connect failed, rc="));
    Serial.println(mqttClient_.state());
  }
}

void CommService::updateMqtt_(const uint32_t nowMs) {
  if (WiFi.status() != WL_CONNECTED) {
    if (mqttWasConnected_) {
      mqttWasConnected_ = false;
      Serial.println(F("[Comm] MQTT disconnected (no WiFi)"));
    }
    return;
  }

  if (mqttClient_.connected()) {
    if (!mqttWasConnected_) {
      mqttWasConnected_ = true;
      Serial.println(F("[Comm] MQTT connected"));
    }
    return;
  }

  if (mqttWasConnected_) {
    mqttWasConnected_ = false;
    Serial.println(F("[Comm] MQTT lost"));
  }

  if (lastMqttAttemptMs_ == 0 ||
      (nowMs - lastMqttAttemptMs_) >= NETWORK_RETRY_INTERVAL_MS) {
    tryMqttConnect_();
    lastMqttAttemptMs_ = nowMs;
  }
}

NetworkStatus CommService::getStatus() const {
  return isWifiConnected() ? NETWORK_ONLINE : NETWORK_OFFLINE;
}

bool CommService::isWifiConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

bool CommService::isMqttConnected() {
  return mqttClient_.connected();
}

void CommService::getDeviceId(char out[13]) const {
  memcpy(out, deviceId_, 13);
}

bool CommService::takeRxPayload(char *out, const size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return false;
  }

  noInterrupts();
  if (!rxPending_) {
    interrupts();
    return false;
  }

  strncpy(out, rxBuffer_, outSize - 1);
  out[outSize - 1] = '\0';
  rxPending_ = false;
  interrupts();
  return true;
}

MqttSysCommand CommService::takePendingSysCommand() {
  noInterrupts();
  if (!sysPending_) {
    interrupts();
    return MqttSysCommand::None;
  }

  char local[sizeof(sysBuffer_)];
  memcpy(local, sysBuffer_, sizeof(local));
  sysPending_ = false;
  interrupts();

  trimInPlace(local);
  Serial.print(F("[Comm] sys command payload="));
  Serial.println(local);

  if (strcmp(local, "BIND_SUCCESS") == 0) {
    return MqttSysCommand::BindSuccess;
  }
  if (strcmp(local, "UNBOUND") == 0) {
    return MqttSysCommand::Unbound;
  }
  return MqttSysCommand::None;
}

void CommService::onMqttMessage(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, topicRx_) == 0) {
    if (!deviceBoundForRx_) {
      Serial.println(F("[Comm] MQTT rx ignored (device not bound)"));
      return;
    }

    size_t n = length;
    if (n > RX_MESSAGE_MAX_LEN) {
      n = RX_MESSAGE_MAX_LEN;
    }

    memcpy(rxBuffer_, payload, n);
    rxBuffer_[n] = '\0';
    rxPending_ = true;

    Serial.print(F("[Comm] MQTT rx len="));
    Serial.print(length);
    Serial.print(F(" text="));
    Serial.println(rxBuffer_);
    return;
  }

  if (strcmp(topic, topicSys_) == 0) {
    size_t n = length;
    if (n >= sizeof(sysBuffer_)) {
      n = sizeof(sysBuffer_) - 1;
    }
    memcpy(sysBuffer_, payload, n);
    sysBuffer_[n] = '\0';
    sysPending_ = true;
    return;
  }

  Serial.print(F("[Comm] MQTT unknown topic="));
  Serial.println(topic);
}
