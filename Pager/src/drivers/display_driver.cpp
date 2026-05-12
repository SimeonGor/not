#include "drivers/display_driver.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

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

void DisplayDriver::drawPairingScreen_(const PagerViewModel &viewModel) {
  drawStatusBar_(F("Status: Offline"));

  display.setTextSize(1);
  display.setCursor(0, 18);
  display.println(F("PAIRING MODE"));

  display.setTextSize(2);
  display.setCursor(16, 34);
  display.print(viewModel.pairingPin);

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(F("Press OK >"));
}

void DisplayDriver::drawIdleScreen_(const PagerViewModel &viewModel) {
  if (viewModel.networkStatus == NETWORK_OFFLINE) {
    drawStatusBar_(F("Status: Offline"));
  } else {
    drawStatusBar_(F("Status: Online"));
  }

  display.setTextSize(1);
  display.setCursor(10, 28);
  display.println(F("NO NEW"));
  display.setCursor(8, 38);
  display.println(F("MESSAGES"));
}

void DisplayDriver::drawReadingScreen_(const PagerViewModel &viewModel) {
  drawStatusBar_(F("READING"));

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (viewModel.messageLayout == nullptr) {
    return;
  }

  const TextLayout &layout = *viewModel.messageLayout;
  for (uint8_t lineIndex = 0; lineIndex < layout.lineCount; lineIndex++) {
    const int16_t lineY =
        CONTENT_TOP_Y + static_cast<int16_t>(lineIndex) * LINE_HEIGHT - viewModel.scrollOffset;

    if (lineY < CONTENT_TOP_Y - LINE_HEIGHT) {
      continue;
    }
    if (lineY > SCREEN_HEIGHT) {
      break;
    }

    display.setCursor(0, lineY);
    display.print(layout.lines[lineIndex]);
  }

  if (layout.maxScrollOffset > 0) {
    display.setCursor(96, 56);
    display.print(viewModel.scrollOffset);
    display.print(F("/"));
    display.print(layout.maxScrollOffset);
  } else {
    display.setCursor(0, 56);
    display.print(F("OK=close"));
  }
}

void DisplayDriver::drawUI(const PagerViewModel &viewModel) {
  display.clearDisplay();

  switch (viewModel.state) {
    case STATE_PAIRING:
      drawPairingScreen_(viewModel);
      break;
    case STATE_IDLE:
      drawIdleScreen_(viewModel);
      break;
    case STATE_READING:
      drawReadingScreen_(viewModel);
      break;
  }

  display.display();
}
