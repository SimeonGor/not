#pragma once

#include "communication/comm_service.h"
#include "drivers/button_driver.h"
#include "drivers/buzzer_driver.h"
#include "drivers/display_driver.h"
#include "pager_types.h"
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

  SystemState state_;
  uint16_t pairingPin_;
  int16_t scrollLine_;
  bool uiDirty_;
  uint32_t lastDisplayUpdateMs_;

  void enterPairingState_();
  void enterIdleState_();
  void enterReadingState_(bool playIncomingBeeps);
  void processLogic_(ButtonEvent event);
  void drawUI_(uint32_t nowMs);
  void applyIncomingMqtt_();
  PagerViewModel buildViewModel_();
};
