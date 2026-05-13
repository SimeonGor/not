#include "business/pager_app.h"

#include <cstring>

#include <ESP.h>

#include "config.h"
#include "services/text_layout_service.h"

// Слой приложения: state machine, опрос кнопок, применение MQTT, троттлинг OLED.

void PagerApp::begin() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println(F("[Pager] boot Smart Retro Pager phase2"));

  commService_.begin();
  Serial.print(F("[Pager] MAC "));
  Serial.println(WiFi.macAddress());

  buttonDriver_.begin();
  buzzerDriver_.begin();
  messageService_.begin();

  randomSeed(ESP.getCycleCount() ^ micros());

  if (!displayDriver_.begin()) {
    Serial.println(F("[Pager] OLED init failed"));
    for (;;) {
      delay(1000);
    }
  }

  enterPairingState_();
  uiDirty_ = true;
  lastDisplayUpdateMs_ = 0;
}

void PagerApp::applyIncomingMqtt_() {
  char rx[RX_MESSAGE_MAX_LEN + 1];
  if (!commService_.takeRxPayload(rx, sizeof(rx))) {
    return;
  }

  messageService_.setIncomingText(rx, RX_MESSAGE_MAX_LEN);
  state_ = STATE_READING;
  scrollLine_ = 0;
  TextLayoutService layoutService;
  scrollLine_ =
      layoutService.clampScrollLine(scrollLine_, messageService_.getLayout().maxScrollLines);
  buzzerDriver_.startIncomingBeep();
  uiDirty_ = true;
}

void PagerApp::loop() {
  const uint32_t nowMs = millis();

  commService_.loop();
  applyIncomingMqtt_();

  const ButtonEvent event = buttonDriver_.readInputs();
  processLogic_(event);

  buzzerDriver_.update(nowMs);

  if (uiDirty_ || (nowMs - lastDisplayUpdateMs_ >= DISPLAY_UPDATE_INTERVAL_MS)) {
    drawUI_(nowMs);
  }
}

void PagerApp::enterPairingState_() {
  state_ = STATE_PAIRING;
  pairingPin_ = static_cast<uint16_t>(random(1000, 10000));
  scrollLine_ = 0;
  uiDirty_ = true;
}

void PagerApp::enterIdleState_() {
  state_ = STATE_IDLE;
  scrollLine_ = 0;
  uiDirty_ = true;
}

void PagerApp::enterReadingState_(const bool playIncomingBeeps) {
  state_ = STATE_READING;
  scrollLine_ = 0;

  TextLayoutService layoutService;
  scrollLine_ = layoutService.clampScrollLine(
      scrollLine_, messageService_.getLayout().maxScrollLines);
  uiDirty_ = true;

  if (playIncomingBeeps) {
    buzzerDriver_.startIncomingBeep();
  }
}

void PagerApp::processLogic_(const ButtonEvent event) {
  if (event == BTN_EVENT_NONE) {
    return;
  }

  switch (state_) {
    case STATE_PAIRING:
      if (event == BTN_EVENT_OK) {
        buzzerDriver_.startClickBeep();
        enterIdleState_();
      }
      break;

    case STATE_IDLE:
      if (event == BTN_EVENT_OK) {
        if (ENABLE_LOCAL_MOCK_MESSAGE) {
          messageService_.loadMockForLocalTest();
          enterReadingState_(true);
        } else {
          buzzerDriver_.startClickBeep();
        }
      }
      break;

    case STATE_READING: {
      buzzerDriver_.startClickBeep();
      TextLayoutService layoutService;
      const int16_t maxLines = messageService_.getLayout().maxScrollLines;

      if (event == BTN_EVENT_UP) {
        scrollLine_ =
            layoutService.clampScrollLine(scrollLine_ - SCROLL_STEP_LINES, maxLines);
        uiDirty_ = true;
      } else if (event == BTN_EVENT_DOWN) {
        scrollLine_ =
            layoutService.clampScrollLine(scrollLine_ + SCROLL_STEP_LINES, maxLines);
        uiDirty_ = true;
      } else if (event == BTN_EVENT_OK) {
        enterIdleState_();
      }
      break;
    }
  }
}

PagerViewModel PagerApp::buildViewModel_() {
  PagerViewModel viewModel = {};
  viewModel.state = state_;
  viewModel.networkStatus = commService_.getStatus();
  viewModel.pairingPin = pairingPin_;
  viewModel.scrollLine = scrollLine_;
  viewModel.messageLayout = &messageService_.getLayout();
  viewModel.wifiConnected = commService_.isWifiConnected();
  viewModel.mqttConnected = commService_.isMqttConnected();
  commService_.getDeviceId(viewModel.deviceId);
  return viewModel;
}

void PagerApp::drawUI_(const uint32_t nowMs) {
  displayDriver_.drawUI(buildViewModel_());
  uiDirty_ = false;
  lastDisplayUpdateMs_ = nowMs;
}
