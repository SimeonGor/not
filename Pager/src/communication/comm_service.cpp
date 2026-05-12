#include "communication/comm_service.h"

#include <cstring>

#include "config.h"

// Слой коммуникации (этап 2): неблокирующий Wi‑Fi и MQTT, топик pager/{deviceId}/rx.
// Переподключение не чаще NETWORK_RETRY_INTERVAL_MS; в loop() вызывается mqttClient.loop().

namespace {
CommService *gComm = nullptr;

void mqttStaticCallback(char *topic, byte *payload, unsigned int length) {
  if (gComm != nullptr) {
    gComm->onMqttMessage(topic, payload, length);
  }
}
}  // namespace

CommService::CommService() : mqttClient_(wifiClient_) {}

void CommService::begin() {
  gComm = this;
  rxBuffer_[0] = '\0';
  rxPending_ = false;
  lastWifiAttemptMs_ = 0;
  lastMqttAttemptMs_ = 0;
  wifiWasConnected_ = false;
  mqttWasConnected_ = false;

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  refreshDeviceId_();
  snprintf(topicRx_, sizeof(topicRx_), "pager/%s/rx", deviceId_);

  mqttClient_.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient_.setBufferSize(512);
  mqttClient_.setCallback(mqttStaticCallback);

  Serial.println(F("[Comm] begin"));
  Serial.print(F("[Comm] deviceId="));
  Serial.println(deviceId_);
  Serial.print(F("[Comm] topicRx="));
  Serial.println(topicRx_);
}

void CommService::refreshDeviceId_() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
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
      Serial.println(F("[Comm] MQTT subscribe failed"));
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

void CommService::onMqttMessage(char *topic, byte *payload, unsigned int length) {
  (void)topic;

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
}
