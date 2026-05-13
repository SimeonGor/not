#include "drivers/display_driver.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include <cstdio>
#include <cstring>

#include "config.h"

namespace {
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}  // namespace

bool DisplayDriver::begin() {
  Wire.begin(PIN_SDA, PIN_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    return false;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  return true;
}

void DisplayDriver::drawStatusBar_(const __FlashStringHelper *statusText) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(statusText);
  display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);
}

void DisplayDriver::drawBootScreen_() {
  drawStatusBar_(F("BOOT"));
  display.setTextSize(1);
  display.setCursor(0, 14);
  display.println(F("Smart Pager"));
  display.setCursor(0, 28);
  display.println(F("Phase 4A"));
}

void DisplayDriver::drawConnectingScreen_() {
  drawStatusBar_(F("NET"));
  display.setTextSize(1);
  display.setCursor(0, 14);
  display.println(F("Connecting WiFi"));
  display.setCursor(0, 28);
  display.println(F("Please wait..."));
}

void DisplayDriver::drawPairingScreen_(const PagerViewModel &viewModel) {
  drawStatusBar_(F("PAIR"));

  display.setTextSize(1);
  display.setCursor(0, 12);
  display.println(F("PAIRING MODE"));

  if (viewModel.pairingRefreshing) {
    display.setCursor(0, 28);
    display.println(F("Refreshing PIN..."));
    return;
  }

  if (viewModel.pairingBackendError || viewModel.pairingPin[0] == '\0') {
    display.setCursor(0, 24);
    display.println(F("PAIRING ERROR"));
    display.setCursor(0, 36);
    display.println(F("Backend offline"));
    display.setCursor(0, 48);
    display.println(F("Retrying..."));
    return;
  }

  display.setCursor(0, 22);
  display.print(F("PIN:"));
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print(viewModel.pairingPin);

  display.setTextSize(1);
  char ttlBuf[12];
  const int rs = viewModel.pairingRemainingSeconds;
  const int m = (rs < 0 ? 0 : rs) / 60;
  const int s = (rs < 0 ? 0 : rs) % 60;
  snprintf(ttlBuf, sizeof(ttlBuf), "%02d:%02d", m, s);
  display.setCursor(0, 48);
  display.print(F("TTL "));
  display.print(ttlBuf);
  display.print(F(" "));
  display.print(TELEGRAM_BOT_HANDLE);
}

void DisplayDriver::drawErrorScreen_() {
  drawStatusBar_(F("ERR"));
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println(F("PAIRING ERROR"));
  display.setCursor(0, 36);
  display.println(F("Backend offline"));
  display.setCursor(0, 48);
  display.println(F("Retrying..."));
}

void DisplayDriver::drawIdleScreen_(const PagerViewModel &viewModel) {
  drawStatusBar_(F("IDLE"));

  display.setTextSize(1);
  display.setCursor(0, 12);
  if (viewModel.bindSuccessFlash) {
    display.println(F("BOUND OK"));
  } else {
    display.println(F("NO NEW MESSAGES"));
  }

  display.setCursor(0, 22);
  display.print(F("WiFi:"));
  display.print(viewModel.wifiConnected ? F("OK") : F("OFF"));

  display.setCursor(64, 22);
  display.print(F("MQTT:"));
  display.print(viewModel.mqttConnected ? F("OK") : F("OFF"));

  display.setCursor(0, 32);
  display.print(F("Back:"));
  display.print(viewModel.backendReachable ? F("OK") : F("OFF"));

  display.setCursor(64, 32);
  display.print(viewModel.isBound ? F("BND") : F("---"));

  display.setCursor(0, 42);
  display.print(F("ID .."));
  display.print(viewModel.deviceIdTail);

  if (viewModel.factoryResetErrorFlash) {
    display.setCursor(0, 52);
    display.print(F("RST: no ACK"));
  } else if (ENABLE_LOCAL_MOCK_MESSAGE) {
    display.setCursor(0, 52);
    display.print(F("OK=mock"));
  }
}

void DisplayDriver::drawReadingScreen_(const PagerViewModel &viewModel) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (viewModel.messageLayout == nullptr) {
    drawStatusBar_(F("READING"));
    return;
  }

  const TextLayout &layout = *viewModel.messageLayout;

  const int32_t totalContentPx =
      static_cast<int32_t>(layout.lineCount) * static_cast<int32_t>(LINE_HEIGHT);
  const int32_t maxScrollPx = totalContentPx - static_cast<int32_t>(MESSAGE_SCROLL_VIEW_HEIGHT);
  int32_t scrollPx = static_cast<int32_t>(viewModel.scrollLine) * static_cast<int32_t>(LINE_HEIGHT);
  if (maxScrollPx > 0 && scrollPx > maxScrollPx) {
    scrollPx = maxScrollPx;
  }
  if (scrollPx < 0) {
    scrollPx = 0;
  }
  const int16_t scrollOffsetPx = static_cast<int16_t>(scrollPx);

  for (uint8_t lineIndex = 0; lineIndex < layout.lineCount; lineIndex++) {
    const int16_t lineY =
        CONTENT_TOP_Y + static_cast<int16_t>(lineIndex) * LINE_HEIGHT - scrollOffsetPx;

    if (lineY < CONTENT_TOP_Y) {
      continue;
    }
    if (lineY + LINE_HEIGHT > READING_FOOTER_TOP_Y) {
      continue;
    }
    if (lineY > SCREEN_HEIGHT) {
      break;
    }

    display.setCursor(0, lineY);
    display.print(layout.lines[lineIndex]);
  }

  drawStatusBar_(F("READING"));

  display.fillRect(0, READING_FOOTER_TOP_Y, SCREEN_WIDTH, SCREEN_HEIGHT - READING_FOOTER_TOP_Y,
                   SSD1306_BLACK);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  if (layout.maxScrollLines > 0) {
    char scrollBuf[20];
    snprintf(scrollBuf, sizeof(scrollBuf), "%d/%d", static_cast<int>(viewModel.scrollLine),
             static_cast<int>(layout.maxScrollLines));
    const size_t slen = strlen(scrollBuf);
    const int16_t charPx = 6;
    int16_t x = static_cast<int16_t>(SCREEN_WIDTH - static_cast<int>(slen) * charPx);
    if (x < 0) {
      x = 0;
    }
    display.setCursor(x, READING_FOOTER_TOP_Y);
    display.print(scrollBuf);
  } else {
    display.setCursor(0, READING_FOOTER_TOP_Y);
    display.print(F("OK=close"));
  }
}

void DisplayDriver::drawUI(const PagerViewModel &viewModel) {
  display.clearDisplay();

  switch (viewModel.state) {
    case STATE_BOOT:
      drawBootScreen_();
      break;
    case STATE_CONNECTING:
      drawConnectingScreen_();
      break;
    case STATE_PAIRING:
      drawPairingScreen_(viewModel);
      break;
    case STATE_IDLE:
      drawIdleScreen_(viewModel);
      break;
    case STATE_READING:
      drawReadingScreen_(viewModel);
      break;
    case STATE_ERROR:
      drawErrorScreen_();
      break;
  }

  display.display();
}
