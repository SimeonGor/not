#pragma once

#include "communication/comm_service.h"
#include "config.h"
#include "drivers/button_driver.h"
#include "drivers/buzzer_driver.h"
#include "drivers/display_driver.h"
#include "pager_types.h"
#include "services/backend_pairing_client.h"
#include "services/message_history.h"
#include "services/message_service.h"

class PagerApp {
 public:
  void begin();
  void loop();

 private:
  ButtonDriver buttonDriver_;
  BuzzerDriver buzzerDriver_;
  DisplayDriver displayDriver_;
  MessageService messageService_;
  CommService commService_;
  BackendPairingClient backend_;
  MessageHistoryStore messageHistory_;

  SystemState state_;
  int16_t scrollLine_;
  bool uiDirty_;
  uint32_t lastDisplayUpdateMs_;

  int historyReadingSlot_{0};

  bool eepromBoundAtBoot_;
  bool isBound_;
  bool backendReachable_;
  bool postWifiSequenceDone_;
  bool pairingBackendError_;

  char currentPin_[PAIRING_PIN_MAX_LEN + 1];
  int pinTtlSeconds_;
  uint32_t pinReceivedAtMillis_;
  bool isRefreshingPin_;

  uint32_t lastPinHttpAttemptMs_;
  bool pinHttpFailBackoff_;

  uint32_t bindSuccessFlashEndMs_;
  uint32_t factoryResetErrorEndMs_;

  uint32_t bothButtonsDownSinceMs_;
  bool factoryResetAwaitRelease_;

  void runPostWifiSequenceOnce_();
  void handleIncomingRxMessage_(const char *msg);
  void applyIncomingRx_();
  void applySysCommands_(uint32_t nowMs);
  void updatePairingPinLifecycle_(uint32_t nowMs);
  bool isPairingPinExpired_(uint32_t nowMs) const;
  int getPairingPinRemainingSeconds_(uint32_t nowMs) const;
  bool canAttemptPinHttp_(uint32_t nowMs, bool ttlExpiredBypassThrottle) const;
  void requestPairingPinFromBackend_(uint32_t nowMs);
  void clearPin_();
  void handleBindSuccess_(uint32_t nowMs);
  void handleUnbound_(uint32_t nowMs);
  void tryFactoryResetHold_(uint32_t nowMs);

  void enterIdleState_();
  void enterReadingState_(bool playIncomingBeeps);
  void enterHistoryListState_();
  void enterHistoryReadingFromList_(int slotIndex);
  void enterErrorState_();

  void scrollMessageUp_();
  void scrollMessageDown_();

  void processLogic_(ButtonEvent event);
  void drawUI_(uint32_t nowMs);
  PagerViewModel buildViewModel_(uint32_t nowMs);
};
