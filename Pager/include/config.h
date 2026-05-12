#pragma once

#include <Arduino.h>

// Пины NodeMCU v3 (см. pager_hardware_phase1_context.md)
static const uint8_t PIN_SCL = D1;
static const uint8_t PIN_SDA = D2;
static const uint8_t PIN_BTN_UP = D5;
static const uint8_t PIN_BTN_DOWN = D6;
static const uint8_t PIN_BTN_OK = D7;
static const uint8_t PIN_BUZZER = D8;

// OLED SSD1306 128x64
static const uint8_t SCREEN_WIDTH = 128;
static const uint8_t SCREEN_HEIGHT = 64;
static const int8_t OLED_RESET = -1;
static const uint8_t OLED_I2C_ADDRESS = 0x3C;

// Текст и прокрутка
static const uint8_t LINE_HEIGHT = 8;
static const int16_t CONTENT_TOP_Y = 15;
static const int16_t CONTENT_VIEW_HEIGHT = SCREEN_HEIGHT - CONTENT_TOP_Y;
static const int16_t SCROLL_STEP_PX = 8;
static const uint8_t MAX_WRAPPED_LINES = 32;
static const uint8_t LINE_CHAR_CAPACITY = 21;

// Ввод и звук
static const uint32_t DEBOUNCE_MS = 150;
static const uint16_t BEEP_SHORT_MS = 60;
static const uint16_t BEEP_MESSAGE_ON_MS = 100;
static const uint16_t BEEP_MESSAGE_OFF_MS = 80;
