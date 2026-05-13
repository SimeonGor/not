#include "services/backend_pairing_client.h"

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ArduinoJson.h>

#include "config.h"

BindingStatusResult BackendPairingClient::fetchBindingStatus(const char *deviceId) const {
  BindingStatusResult r = {};
  r.httpOk = false;
  r.bound = false;
  r.httpCode = -1;

  char url[160];
  snprintf(url, sizeof(url), "http://%s:%d/api/v1/devices/%s/binding-status", BACKEND_HOST,
           BACKEND_PORT, deviceId);

  Serial.print(F("[HTTP] GET binding-status URL="));
  Serial.println(url);

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(3000);
  if (!http.begin(client, url)) {
    Serial.println(F("[HTTP] begin failed"));
    return r;
  }

  const int code = http.GET();
  r.httpCode = code;
  Serial.print(F("[HTTP] binding-status code="));
  Serial.println(code);

  if (code < 200 || code >= 300) {
    http.end();
    return r;
  }

  const String body = http.getString();
  http.end();

  Serial.print(F("[HTTP] binding-status body="));
  Serial.println(body);

  StaticJsonDocument<384> doc;
  const DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.print(F("[HTTP] binding-status JSON err="));
    Serial.println(err.c_str());
    return r;
  }

  r.bound = doc["bound"].as<bool>();
  r.httpOk = true;
  return r;
}

PairingPinResult BackendPairingClient::requestPairingPin(const char *deviceId) const {
  PairingPinResult r = {};
  r.httpOk = false;
  r.pin[0] = '\0';
  r.ttlSeconds = 0;
  r.httpCode = -1;

  char url[160];
  snprintf(url, sizeof(url), "http://%s:%d/api/v1/devices/%s/pairing-pin", BACKEND_HOST,
           BACKEND_PORT, deviceId);

  Serial.print(F("[HTTP] POST pairing-pin URL="));
  Serial.println(url);

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(3000);
  if (!http.begin(client, url)) {
    Serial.println(F("[HTTP] begin failed (pairing-pin)"));
    return r;
  }

  http.addHeader(F("Content-Type"), F("application/json"));
  const int code = http.POST("{}");
  r.httpCode = code;
  Serial.print(F("[HTTP] pairing-pin code="));
  Serial.println(code);

  if (code < 200 || code >= 300) {
    http.end();
    return r;
  }

  const String body = http.getString();
  http.end();

  Serial.print(F("[HTTP] pairing-pin body="));
  Serial.println(body);

  StaticJsonDocument<384> doc;
  const DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.print(F("[HTTP] pairing-pin JSON err="));
    Serial.println(err.c_str());
    return r;
  }

  const char *pinStr = doc["pin"].as<const char *>();
  if (pinStr == nullptr) {
    Serial.println(F("[HTTP] pairing-pin missing pin"));
    return r;
  }

  strncpy(r.pin, pinStr, sizeof(r.pin) - 1);
  r.pin[sizeof(r.pin) - 1] = '\0';
  r.ttlSeconds = doc["ttlSeconds"].as<int>();
  if (r.ttlSeconds <= 0) {
    r.ttlSeconds = 300;
  }

  Serial.print(F("[HTTP] pairing-pin received pin len="));
  Serial.print(strlen(r.pin));
  Serial.print(F(" ttlSeconds="));
  Serial.println(r.ttlSeconds);

  r.httpOk = true;
  return r;
}

ResetBindingResult BackendPairingClient::postResetBinding(const char *deviceId) const {
  ResetBindingResult r = {};
  r.httpOk = false;
  r.httpCode = -1;

  char url[160];
  snprintf(url, sizeof(url), "http://%s:%d/api/v1/devices/%s/reset-binding", BACKEND_HOST,
           BACKEND_PORT, deviceId);

  Serial.print(F("[HTTP] POST reset-binding URL="));
  Serial.println(url);

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(3000);
  if (!http.begin(client, url)) {
    Serial.println(F("[HTTP] begin failed (reset-binding)"));
    return r;
  }

  http.addHeader(F("Content-Type"), F("application/json"));
  const int code = http.POST("{}");
  r.httpCode = code;
  Serial.print(F("[HTTP] reset-binding code="));
  Serial.println(code);

  if (code >= 200 && code < 300) {
    r.httpOk = true;
  }
  http.end();
  return r;
}
