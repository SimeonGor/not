#include "drivers/display_driver.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

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

void DisplayDriver::drawPairingScreen_(const PagerViewModel &viewModel) {
  drawStatusBar_(F("PAIRING"));

  display.setTextSize(1);
  display.setCursor(0, 12);
  display.println(F("PAIRING MODE"));

  display.setTextSize(2);
  display.setCursor(20, 24);
  display.print(viewModel.pairingPin);

  display.setTextSize(1);
  display.setCursor(0, 44);
  display.print(F("ID:"));
  display.print(viewModel.deviceId);

  display.setCursor(0, 54);
  display.print(F("OK=next"));
}

void DisplayDriver::drawIdleScreen_(const PagerViewModel &viewModel) {
  drawStatusBar_(F("IDLE"));

  display.setTextSize(1);
  display.setCursor(0, 12);
  display.println(F("NO NEW MESSAGES"));

  display.setCursor(0, 22);
  display.print(F("WiFi:"));
  display.print(viewModel.wifiConnected ? F("OK") : F("OFF"));

  display.setCursor(0, 32);
  display.print(F("MQTT:"));
  display.print(viewModel.mqttConnected ? F("OK") : F("OFF"));

  display.setCursor(0, 42);
  display.print(F("ID .."));
  const char *id = viewModel.deviceId;
  const size_t len = strlen(id);
  if (len >= 4) {
    display.print(id + len - 4);
  } else {
    display.print(id);
  }

  if (ENABLE_LOCAL_MOCK_MESSAGE) {
    display.setCursor(0, 54);
    display.print(F("OK=mock"));
  }
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
