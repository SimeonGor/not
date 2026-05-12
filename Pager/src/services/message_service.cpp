#include "services/message_service.h"

#include "services/text_layout_service.h"

const char MessageService::MOCK_MESSAGE[] PROGMEM =
    "Alex: Urgent meeting at 18:00 in room 402. Bring the thermal report and "
    "the spare NodeMCU from the lab shelf. If you are delayed, reply via the "
    "bot when Wi-Fi is up. Confirm with OK on the pager.";

void MessageService::begin() {
  TextLayoutService layoutService;
  layout_ = layoutService.wrapProgmemText(MOCK_MESSAGE);
}

const TextLayout &MessageService::getLayout() const {
  return layout_;
}
