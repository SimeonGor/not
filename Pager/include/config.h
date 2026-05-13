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
// В READING с y=56 — полоса счётчика/подсказки; текст не заходит ниже
static const int16_t READING_FOOTER_TOP_Y = 56;
// Высота области прокрутки текста (для maxScrollLines в TextLayoutService)
static const int16_t MESSAGE_SCROLL_VIEW_HEIGHT = READING_FOOTER_TOP_Y - CONTENT_TOP_Y;
/** Шаг кнопок вверх/вниз в режиме чтения: целые строки. */
static const int16_t SCROLL_STEP_LINES = 1;
static const uint8_t MAX_WRAPPED_LINES = 32;
static const uint8_t LINE_CHAR_CAPACITY = 21;

// Ввод (антидребезг)
static const uint32_t DEBOUNCE_MS = 150;

// --- Сеть (этап 2): MQTT_HOST — LAN IP машины с Docker/Mosquitto, не localhost ---
static constexpr const char *WIFI_SSID = "YOUR_WIFI_NAME";
static constexpr const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static constexpr const char *MQTT_HOST = "192.168.1.45";
static constexpr int MQTT_PORT = 1883;

// --- Backend Phase 4A (HTTP pairing). NodeMCU cannot use "localhost" — use the LAN IP of the PC
// where Spring Boot runs (same Wi‑Fi as the device). ---
static constexpr const char *BACKEND_HOST = "192.168.1.45";
static constexpr int BACKEND_PORT = 8080;

/** Подпись бота на экране привязки (Telegram). */
static constexpr const char *TELEGRAM_BOT_HANDLE = "@YourBot";

static constexpr uint32_t PIN_REFRESH_RETRY_INTERVAL_MS = 10000;
static constexpr uint32_t FACTORY_RESET_HOLD_MS = 5000;
static constexpr uint32_t BIND_SUCCESS_FLASH_MS = 2000;

// В IDLE: OK подставляет mock только при true (тест без брокера)
static constexpr bool ENABLE_LOCAL_MOCK_MESSAGE = false;

static constexpr uint8_t HISTORY_SIZE = 5;
/** Макс. длина одного сообщения в истории (совпадает с rx payload). */
static constexpr size_t HISTORY_MESSAGE_MAX_LEN = 256;
/** Длина превью в списке истории (символы до «...»). */
static constexpr int HISTORY_PREVIEW_MAX_LEN = 18;
/** Буфер одной строки списка на OLED (+ маркер и '\0'). */
static constexpr size_t HISTORY_LIST_ROW_BUF = 24;

static constexpr uint32_t NETWORK_RETRY_INTERVAL_MS = 5000;
static constexpr uint32_t DISPLAY_UPDATE_INTERVAL_MS = 150;
static constexpr size_t RX_MESSAGE_MAX_LEN = 256;
static constexpr size_t PAIRING_PIN_MAX_LEN = 6;
