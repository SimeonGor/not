#pragma once

#include <Arduino.h>

#include "config.h"

struct BindingStatusResult {
  bool httpOk;
  int httpCode;
  bool bound;
};

struct PairingPinResult {
  bool httpOk;
  int httpCode;
  char pin[PAIRING_PIN_MAX_LEN + 1];
  int ttlSeconds;
};

struct ResetBindingResult {
  bool httpOk;
  int httpCode;
};

class BackendPairingClient {
 public:
  BindingStatusResult fetchBindingStatus(const char *deviceId) const;
  PairingPinResult requestPairingPin(const char *deviceId) const;
  ResetBindingResult postResetBinding(const char *deviceId) const;
};
