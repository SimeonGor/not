#include "business/pager_app.h"

#include <ESP.h>

#include "config.h"
#include "services/text_layout_service.h"

void PagerApp::begin() {
  buttonDriver_.begin();
  buzzerDriver_.begin();
  messageService_.begin();
  commService_.begin();

  randomSeed(ESP.getCycleCount() ^ micros());

  if (!displayDriver_.begin()) {
    for (;;) {
      delay(1000);
    }
  }

  enterPairingState_();
}

void PagerApp::loop() {
  commService_.loop();

  const ButtonEvent event = buttonDriver_.readInputs();
  processLogic_(event);
  drawUI_();
}

void PagerApp::enterPairingState_() {
  state_ = STATE_PAIRING;
  pairingPin_ = static_cast<uint16_t>(random(1000, 10000));
  scrollOffset_ = 0;
  uiDirty_ = true;
}

void PagerApp::enterIdleState_() {
  state_ = STATE_IDLE;
  scrollOffset_ = 0;
  uiDirty_ = true;
}

void PagerApp::enterReadingState_(const bool playIncomingBeeps) {
  state_ = STATE_READING;
  scrollOffset_ = 0;

  TextLayoutService layoutService;
  scrollOffset_ = layoutService.clampScrollOffset(
      scrollOffset_, messageService_.getLayout().maxScrollOffset);
  uiDirty_ = true;

  if (playIncomingBeeps) {
    buzzerDriver_.beepIncomingMessage();
  }
}

void PagerApp::processLogic_(const ButtonEvent event) {
  if (commService_.consumeBindSuccess()) {
    enterIdleState_();
    return;
  }

  if (event == BTN_EVENT_NONE) {
    return;
  }

  switch (state_) {
    case STATE_PAIRING:
      if (event == BTN_EVENT_OK) {
        buzzerDriver_.beep(BEEP_SHORT_MS);
        enterIdleState_();
      }
      break;

    case STATE_IDLE:
      if (event == BTN_EVENT_OK) {
        enterReadingState_(true);
      }
      break;

    case STATE_READING: {
      buzzerDriver_.beep(BEEP_SHORT_MS);
      TextLayoutService layoutService;
      const int16_t maxScroll = messageService_.getLayout().maxScrollOffset;

      if (event == BTN_EVENT_UP) {
        scrollOffset_ = layoutService.clampScrollOffset(scrollOffset_ - SCROLL_STEP_PX, maxScroll);
        uiDirty_ = true;
      } else if (event == BTN_EVENT_DOWN) {
        scrollOffset_ = layoutService.clampScrollOffset(scrollOffset_ + SCROLL_STEP_PX, maxScroll);
        uiDirty_ = true;
      } else if (event == BTN_EVENT_OK) {
        enterIdleState_();
      }
      break;
    }
  }
}

PagerViewModel PagerApp::buildViewModel_() const {
  PagerViewModel viewModel = {};
  viewModel.state = state_;
  viewModel.networkStatus = commService_.getStatus();
  viewModel.pairingPin = pairingPin_;
  viewModel.scrollOffset = scrollOffset_;
  viewModel.messageLayout = &messageService_.getLayout();
  return viewModel;
}

void PagerApp::drawUI_() {
  if (!uiDirty_) {
    return;
  }

  displayDriver_.drawUI(buildViewModel_());
  uiDirty_ = false;
}
