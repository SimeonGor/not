#include "services/message_service.h"

#include <cstring>

#include <pgmspace.h>

#include "services/text_layout_service.h"

const char MessageService::MOCK_MESSAGE[] PROGMEM =
    "Alex: Urgent meeting at 18:00 in room 402. Bring the thermal report and "
    "the spare NodeMCU from the lab shelf. If you are delayed, reply via the "
    "bot when Wi-Fi is up. Confirm with OK on the pager.";

void MessageService::begin() {
  clearToEmpty();
}

const TextLayout &MessageService::getLayout() const {
  return layout_;
}

void MessageService::clearToEmpty() {
  rawBuffer_[0] = '\0';
  TextLayoutService layoutService;
  layout_ = layoutService.wrapRamText(rawBuffer_);
}

void MessageService::setIncomingText(const char *text, const size_t maxLen) {
  if (text == nullptr) {
    clearToEmpty();
    return;
  }

  const size_t cap = min(maxLen, sizeof(rawBuffer_) - 1);
  size_t i = 0;
  for (; i < cap && text[i] != '\0'; ++i) {
    rawBuffer_[i] = text[i];
  }
  rawBuffer_[i] = '\0';

  TextLayoutService layoutService;
  layout_ = layoutService.wrapRamText(rawBuffer_);
}

void MessageService::loadMockForLocalTest() {
  strncpy_P(rawBuffer_, MOCK_MESSAGE, sizeof(rawBuffer_) - 1);
  rawBuffer_[sizeof(rawBuffer_) - 1] = '\0';

  TextLayoutService layoutService;
  layout_ = layoutService.wrapRamText(rawBuffer_);
}
