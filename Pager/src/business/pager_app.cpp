#include "business/pager_app.h"

#include <EEPROM.h>
#include <cstring>

#include <ESP8266WiFi.h>
#include <ESP.h>

#include "config.h"
#include "services/eeprom_binding_store.h"
#include "services/message_history.h"
#include "services/text_layout_service.h"

void PagerApp::begin() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println(F("[Pager] boot Smart Retro Pager phase5a"));

  EEPROM.begin(EEPROM_SIZE);
  eepromBoundAtBoot_ = loadBoundStateFromEEPROM();

  commService_.begin();
  messageHistory_.begin();
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

  state_ = STATE_CONNECTING;
  scrollLine_ = 0;
  uiDirty_ = true;
  lastDisplayUpdateMs_ = 0;

  isBound_ = false;
  backendReachable_ = false;
  postWifiSequenceDone_ = false;
  pairingBackendError_ = false;
  clearPin_();
  pinTtlSeconds_ = 0;
  pinReceivedAtMillis_ = 0;
  isRefreshingPin_ = false;
  lastPinHttpAttemptMs_ = 0;
  pinHttpFailBackoff_ = false;
  bindSuccessFlashEndMs_ = 0;
  factoryResetErrorEndMs_ = 0;
  bothButtonsDownSinceMs_ = 0;
  factoryResetAwaitRelease_ = false;

  commService_.setDeviceBoundForRx(false);
}

void PagerApp::clearPin_() {
  currentPin_[0] = '\0';
  pinTtlSeconds_ = 0;
  pinReceivedAtMillis_ = 0;
}

bool PagerApp::isPairingPinExpired_(const uint32_t nowMs) const {
  if (currentPin_[0] == '\0' || pinTtlSeconds_ <= 0) {
    return false;
  }
  const uint32_t elapsedMs = nowMs - pinReceivedAtMillis_;
  return elapsedMs >= static_cast<uint32_t>(pinTtlSeconds_) * 1000UL;
}

int PagerApp::getPairingPinRemainingSeconds_(const uint32_t nowMs) const {
  if (currentPin_[0] == '\0' || pinTtlSeconds_ <= 0) {
    return 0;
  }
  const uint32_t elapsedMs = nowMs - pinReceivedAtMillis_;
  const uint32_t ttlMs = static_cast<uint32_t>(pinTtlSeconds_) * 1000UL;
  if (elapsedMs >= ttlMs) {
    return 0;
  }
  return static_cast<int>((ttlMs - elapsedMs) / 1000UL);
}

bool PagerApp::canAttemptPinHttp_(const uint32_t nowMs,
                                  const bool ttlExpiredBypassThrottle) const {
  if (ttlExpiredBypassThrottle) {
    return true;
  }
  if (!pinHttpFailBackoff_) {
    return true;
  }
  return (nowMs - lastPinHttpAttemptMs_) >= PIN_REFRESH_RETRY_INTERVAL_MS;
}

void PagerApp::requestPairingPinFromBackend_(const uint32_t nowMs) {
  char deviceId[13];
  commService_.getDeviceId(deviceId);

  const bool ttlExpired = isPairingPinExpired_(nowMs) && currentPin_[0] != '\0';
  if (!canAttemptPinHttp_(nowMs, ttlExpired)) {
    return;
  }

  if (state_ != STATE_READING && state_ != STATE_HISTORY_READING &&
      state_ != STATE_HISTORY_LIST) {
    state_ = STATE_PAIRING;
    uiDirty_ = true;
  }

  isRefreshingPin_ = true;
  uiDirty_ = true;
  lastPinHttpAttemptMs_ = nowMs;

  Serial.println(F("[HTTP] pairing-pin request (device flow)"));

  const PairingPinResult r = backend_.requestPairingPin(deviceId);
  isRefreshingPin_ = false;

  if (r.httpOk) {
    pinHttpFailBackoff_ = false;
    strncpy(currentPin_, r.pin, sizeof(currentPin_) - 1);
    currentPin_[sizeof(currentPin_) - 1] = '\0';
    pinTtlSeconds_ = r.ttlSeconds;
    pinReceivedAtMillis_ = millis();
    pairingBackendError_ = false;
    state_ = STATE_PAIRING;
    Serial.print(F("[Pager] PIN received ttlSeconds="));
    Serial.println(pinTtlSeconds_);
  } else {
    pinHttpFailBackoff_ = true;
    clearPin_();
    pairingBackendError_ = true;
    state_ = STATE_ERROR;
    Serial.println(F("[Pager] pairing-pin failed (backoff 10s)"));
  }
  uiDirty_ = true;
}

void PagerApp::runPostWifiSequenceOnce_() {
  char deviceId[13];
  commService_.getDeviceId(deviceId);

  const BindingStatusResult st = backend_.fetchBindingStatus(deviceId);
  postWifiSequenceDone_ = true;

  if (st.httpOk) {
    backendReachable_ = true;
    isBound_ = st.bound;
    saveBoundStateToEEPROM(isBound_);
    commService_.setDeviceBoundForRx(isBound_);

    if (isBound_) {
      clearPin_();
      pairingBackendError_ = false;
      enterIdleState_();
      Serial.println(F("[Pager] boot: bound=true -> IDLE"));
      return;
    }

    Serial.println(F("[Pager] boot: bound=false -> request PIN"));
    requestPairingPinFromBackend_(millis());
    return;
  }

  backendReachable_ = false;
  Serial.println(F("[Pager] boot: binding-status failed, using EEPROM fallback"));

  if (eepromBoundAtBoot_) {
    isBound_ = true;
    commService_.setDeviceBoundForRx(true);
    clearPin_();
    pairingBackendError_ = false;
    enterIdleState_();
    Serial.println(F("[Pager] boot: EEPROM bound -> IDLE Backend OFF"));
    return;
  }

  isBound_ = false;
  commService_.setDeviceBoundForRx(false);
  requestPairingPinFromBackend_(millis());
  if (currentPin_[0] == '\0') {
    enterErrorState_();
    Serial.println(F("[Pager] boot: unbound + backend offline -> ERROR/retry"));
  }
}

void PagerApp::enterIdleState_() {
  state_ = STATE_IDLE;
  scrollLine_ = 0;
  uiDirty_ = true;
}

void PagerApp::enterHistoryListState_() {
  state_ = STATE_HISTORY_LIST;
  scrollLine_ = 0;
  uiDirty_ = true;
}

void PagerApp::enterHistoryReadingFromList_(const int slotIndex) {
  const String text = messageHistory_.messageAt(slotIndex);
  messageService_.setIncomingText(text.c_str(), text.length());
  scrollLine_ = 0;
  TextLayoutService layoutService;
  scrollLine_ =
      layoutService.clampScrollLine(scrollLine_, messageService_.getLayout().maxScrollLines);
  historyReadingSlot_ = slotIndex;
  state_ = STATE_HISTORY_READING;
  uiDirty_ = true;
}

void PagerApp::scrollMessageUp_() {
  TextLayoutService layoutService;
  const int16_t maxLines = messageService_.getLayout().maxScrollLines;
  scrollLine_ = layoutService.clampScrollLine(scrollLine_ - SCROLL_STEP_LINES, maxLines);
  uiDirty_ = true;
}

void PagerApp::scrollMessageDown_() {
  TextLayoutService layoutService;
  const int16_t maxLines = messageService_.getLayout().maxScrollLines;
  scrollLine_ = layoutService.clampScrollLine(scrollLine_ + SCROLL_STEP_LINES, maxLines);
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

void PagerApp::enterErrorState_() {
  state_ = STATE_ERROR;
  scrollLine_ = 0;
  uiDirty_ = true;
}

void PagerApp::handleIncomingRxMessage_(const char *msg) {
  if (msg == nullptr) {
    return;
  }

  String m(msg);
  if (m.length() > static_cast<int>(HISTORY_MESSAGE_MAX_LEN)) {
    m = m.substring(0, HISTORY_MESSAGE_MAX_LEN);
  }

  messageHistory_.addMessageToHistory(m);
  messageService_.setIncomingText(m.c_str(), m.length());

  enterReadingState_(true);

  TextLayoutService layoutService;
  scrollLine_ =
      layoutService.clampScrollLine(scrollLine_, messageService_.getLayout().maxScrollLines);
  uiDirty_ = true;
}

void PagerApp::applyIncomingRx_() {
  char rx[RX_MESSAGE_MAX_LEN + 1];
  if (!commService_.takeRxPayload(rx, sizeof(rx))) {
    return;
  }

  handleIncomingRxMessage_(rx);
}

void PagerApp::handleBindSuccess_(const uint32_t nowMs) {
  saveBoundStateToEEPROM(true);
  isBound_ = true;
  commService_.setDeviceBoundForRx(true);
  clearPin_();
  pairingBackendError_ = false;
  pinHttpFailBackoff_ = false;
  enterIdleState_();
  bindSuccessFlashEndMs_ = nowMs + BIND_SUCCESS_FLASH_MS;
  buzzerDriver_.startSuccessBeep();
  uiDirty_ = true;
  Serial.println(F("[Pager] BIND_SUCCESS -> IDLE, EEPROM true"));
}

void PagerApp::handleUnbound_(const uint32_t nowMs) {
  Serial.println(F("[Pager] UNBOUND from MQTT"));
  saveBoundStateToEEPROM(false);
  isBound_ = false;
  commService_.setDeviceBoundForRx(false);
  clearPin_();
  messageHistory_.begin();
  requestPairingPinFromBackend_(nowMs);
  if (currentPin_[0] != '\0') {
    state_ = STATE_PAIRING;
  } else {
    enterErrorState_();
  }
  uiDirty_ = true;
}

void PagerApp::applySysCommands_(const uint32_t nowMs) {
  const MqttSysCommand cmd = commService_.takePendingSysCommand();
  if (cmd == MqttSysCommand::None) {
    return;
  }

  if (cmd == MqttSysCommand::BindSuccess) {
    handleBindSuccess_(nowMs);
    return;
  }
  if (cmd == MqttSysCommand::Unbound) {
    handleUnbound_(nowMs);
  }
}

void PagerApp::updatePairingPinLifecycle_(const uint32_t nowMs) {
  if (isBound_ || state_ == STATE_READING || state_ == STATE_HISTORY_READING) {
    return;
  }
  if (state_ != STATE_PAIRING && state_ != STATE_ERROR) {
    return;
  }

  if (currentPin_[0] != '\0' && !isPairingPinExpired_(nowMs)) {
    return;
  }

  if (currentPin_[0] != '\0' && isPairingPinExpired_(nowMs)) {
    Serial.println(F("[Pager] PIN expired, refreshing"));
  }

  const bool ttlExpired = (currentPin_[0] != '\0') && isPairingPinExpired_(nowMs);
  if (!canAttemptPinHttp_(nowMs, ttlExpired)) {
    return;
  }

  requestPairingPinFromBackend_(nowMs);
}

void PagerApp::tryFactoryResetHold_(const uint32_t nowMs) {
  const bool allow = (state_ == STATE_IDLE || state_ == STATE_PAIRING || state_ == STATE_ERROR ||
                      state_ == STATE_HISTORY_LIST || state_ == STATE_HISTORY_READING);
  if (!allow) {
    bothButtonsDownSinceMs_ = 0;
    return;
  }

  const bool rawBoth =
      digitalRead(PIN_BTN_UP) == LOW && digitalRead(PIN_BTN_DOWN) == LOW;

  if (factoryResetAwaitRelease_) {
    if (!rawBoth) {
      factoryResetAwaitRelease_ = false;
    }
    return;
  }

  if (rawBoth) {
    if (bothButtonsDownSinceMs_ == 0) {
      bothButtonsDownSinceMs_ = nowMs;
    } else if (nowMs - bothButtonsDownSinceMs_ >= FACTORY_RESET_HOLD_MS) {
      bothButtonsDownSinceMs_ = 0;
      factoryResetAwaitRelease_ = true;

      char deviceId[13];
      commService_.getDeviceId(deviceId);
      Serial.println(F("[Pager] factory reset: UP+DOWN 5s -> POST reset-binding"));

      const ResetBindingResult rr = backend_.postResetBinding(deviceId);
      if (rr.httpOk) {
        saveBoundStateToEEPROM(false);
        isBound_ = false;
        commService_.setDeviceBoundForRx(false);
        clearPin_();
        pinHttpFailBackoff_ = false;
        requestPairingPinFromBackend_(nowMs);
        if (currentPin_[0] != '\0') {
          state_ = STATE_PAIRING;
        } else {
          enterErrorState_();
        }
        Serial.println(F("[Pager] factory reset: backend OK, new PIN flow"));
      } else {
        Serial.println(
            F("[Pager] factory reset: backend did NOT confirm reset — saving EEPROM false "
              "locally with warning"));
        saveBoundStateToEEPROM(false);
        isBound_ = false;
        commService_.setDeviceBoundForRx(false);
        factoryResetErrorEndMs_ = nowMs + 3000;
        requestPairingPinFromBackend_(nowMs);
        if (currentPin_[0] != '\0') {
          state_ = STATE_PAIRING;
        } else {
          enterErrorState_();
        }
      }
      uiDirty_ = true;
    }
  } else {
    bothButtonsDownSinceMs_ = 0;
  }
}

void PagerApp::loop() {
  const uint32_t nowMs = millis();

  commService_.loop();

  if (commService_.isWifiConnected() && !postWifiSequenceDone_) {
    runPostWifiSequenceOnce_();
  }

  applyIncomingRx_();
  applySysCommands_(nowMs);

  updatePairingPinLifecycle_(nowMs);
  tryFactoryResetHold_(nowMs);

  const ButtonEvent event = buttonDriver_.readInputs();
  processLogic_(event);

  buzzerDriver_.update(nowMs);

  if (uiDirty_ || (nowMs - lastDisplayUpdateMs_ >= DISPLAY_UPDATE_INTERVAL_MS)) {
    drawUI_(nowMs);
  }
}

void PagerApp::processLogic_(const ButtonEvent event) {
  if (event == BTN_EVENT_NONE) {
    return;
  }

  switch (state_) {
    case STATE_BOOT:
    case STATE_CONNECTING:
      break;

    case STATE_PAIRING:
      if (event == BTN_EVENT_OK) {
        buzzerDriver_.startClickBeep();
      }
      break;

    case STATE_IDLE:
      if (event == BTN_EVENT_OK) {
        if (ENABLE_LOCAL_MOCK_MESSAGE) {
          messageService_.loadMockForLocalTest();
          enterReadingState_(true);
        } else if (messageHistory_.count() > 0) {
          messageHistory_.setSelectedHistoryIndex(0);
          enterHistoryListState_();
          Serial.println(F("[HISTORY] Open list"));
        } else {
          buzzerDriver_.startClickBeep();
          Serial.println(F("[HISTORY] Empty"));
        }
      }
      break;

    case STATE_ERROR:
      if (event == BTN_EVENT_OK) {
        buzzerDriver_.startClickBeep();
      }
      break;

    case STATE_READING:
      if (event == BTN_EVENT_UP) {
        scrollMessageUp_();
        buzzerDriver_.startClickBeep();
      } else if (event == BTN_EVENT_DOWN) {
        scrollMessageDown_();
        buzzerDriver_.startClickBeep();
      } else if (event == BTN_EVENT_OK) {
        buzzerDriver_.startClickBeep();
        enterIdleState_();
      }
      break;

    case STATE_HISTORY_LIST:
      if (event == BTN_EVENT_UP) {
        messageHistory_.setSelectedHistoryIndex(messageHistory_.selectedHistoryIndex() - 1);
        Serial.print(F("[HISTORY] Selected index="));
        Serial.println(messageHistory_.selectedHistoryIndex());
        buzzerDriver_.startClickBeep();
        uiDirty_ = true;
      } else if (event == BTN_EVENT_DOWN) {
        messageHistory_.setSelectedHistoryIndex(messageHistory_.selectedHistoryIndex() + 1);
        Serial.print(F("[HISTORY] Selected index="));
        Serial.println(messageHistory_.selectedHistoryIndex());
        buzzerDriver_.startClickBeep();
        uiDirty_ = true;
      } else if (event == BTN_EVENT_OK) {
        if (messageHistory_.count() > 0) {
          const int slot = messageHistory_.selectedHistoryIndex();
          enterHistoryReadingFromList_(slot);
          Serial.print(F("[HISTORY] Open message index="));
          Serial.println(slot + 1);
          buzzerDriver_.startClickBeep();
        }
      }
      break;

    case STATE_HISTORY_READING:
      if (event == BTN_EVENT_UP) {
        scrollMessageUp_();
        buzzerDriver_.startClickBeep();
      } else if (event == BTN_EVENT_DOWN) {
        scrollMessageDown_();
        buzzerDriver_.startClickBeep();
      } else if (event == BTN_EVENT_OK) {
        buzzerDriver_.startClickBeep();
        enterIdleState_();
      }
      break;
  }
}

PagerViewModel PagerApp::buildViewModel_(const uint32_t nowMs) {
  PagerViewModel viewModel = {};
  viewModel.state = state_;
  viewModel.networkStatus = commService_.getStatus();
  viewModel.scrollLine = scrollLine_;
  viewModel.messageLayout = &messageService_.getLayout();
  viewModel.wifiConnected = commService_.isWifiConnected();
  viewModel.mqttConnected = commService_.isMqttConnected();
  commService_.getDeviceId(viewModel.deviceId);

  memset(viewModel.pairingPin, 0, sizeof(viewModel.pairingPin));
  strncpy(viewModel.pairingPin, currentPin_, sizeof(viewModel.pairingPin) - 1);

  viewModel.pairingRemainingSeconds = getPairingPinRemainingSeconds_(nowMs);
  viewModel.pairingRefreshing = isRefreshingPin_;
  viewModel.pairingBackendError = pairingBackendError_;
  viewModel.backendReachable = backendReachable_;
  viewModel.isBound = isBound_;
  viewModel.bindSuccessFlash =
      (bindSuccessFlashEndMs_ != 0) && (static_cast<int32_t>(nowMs - bindSuccessFlashEndMs_) < 0);
  viewModel.factoryResetErrorFlash =
      (factoryResetErrorEndMs_ != 0) &&
      (static_cast<int32_t>(nowMs - factoryResetErrorEndMs_) < 0);

  const char *id = viewModel.deviceId;
  const size_t len = strlen(id);
  memset(viewModel.deviceIdTail, 0, sizeof(viewModel.deviceIdTail));
  if (len >= 6) {
    strncpy(viewModel.deviceIdTail, id + len - 6, sizeof(viewModel.deviceIdTail) - 1);
  } else {
    strncpy(viewModel.deviceIdTail, id, sizeof(viewModel.deviceIdTail) - 1);
  }

  const int hc = messageHistory_.count();
  viewModel.histCount = static_cast<uint8_t>(hc < 0 ? 0 : (hc > 255 ? 255 : hc));
  int selIdx = messageHistory_.selectedHistoryIndex();
  if (selIdx < 0) {
    selIdx = 0;
  }
  if (selIdx > 255) {
    selIdx = 255;
  }
  viewModel.histSelected = static_cast<uint8_t>(selIdx);

  viewModel.historyReadingMode = (state_ == STATE_HISTORY_READING);
  if (state_ == STATE_HISTORY_READING && hc > 0) {
    viewModel.histReadX = static_cast<uint8_t>(historyReadingSlot_ + 1);
    viewModel.histReadY = static_cast<uint8_t>(hc > 255 ? 255 : hc);
  } else {
    viewModel.histReadX = 0;
    viewModel.histReadY = 0;
  }

  memset(viewModel.histListRow0, 0, sizeof(viewModel.histListRow0));
  memset(viewModel.histListRow1, 0, sizeof(viewModel.histListRow1));
  memset(viewModel.histListRow2, 0, sizeof(viewModel.histListRow2));

  if (state_ == STATE_HISTORY_LIST && hc > 0) {
    const int sel = messageHistory_.selectedHistoryIndex();
    int windowStart = sel;
    if (hc > 3 && sel > hc - 3) {
      windowStart = hc - 3;
    }
    for (int r = 0; r < 3; ++r) {
      char *dst =
          (r == 0) ? viewModel.histListRow0 : (r == 1) ? viewModel.histListRow1 : viewModel.histListRow2;
      const int idx = windowStart + r;
      if (idx >= hc) {
        continue;
      }
      dst[0] = (idx == sel) ? '>' : ' ';
      const String prev =
          MessageHistoryStore::makeMessagePreview(messageHistory_.messageAt(idx), HISTORY_PREVIEW_MAX_LEN);
      strncpy(dst + 1, prev.c_str(), HISTORY_LIST_ROW_BUF - 2);
      dst[HISTORY_LIST_ROW_BUF - 1] = '\0';
    }
  }

  return viewModel;
}

void PagerApp::drawUI_(const uint32_t nowMs) {
  const PagerViewModel vm = buildViewModel_(nowMs);
  displayDriver_.drawUI(vm);
  uiDirty_ = false;
  lastDisplayUpdateMs_ = nowMs;
}
