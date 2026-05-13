#pragma once

#include <Arduino.h>

// Локальный кэш привязки (backend — источник истины).
#define EEPROM_SIZE 64
#define EEPROM_MAGIC_ADDR 0
#define EEPROM_BOUND_ADDR 1
#define EEPROM_MAGIC_VALUE 0x42
#define EEPROM_BOUND_TRUE 0x01
#define EEPROM_BOUND_FALSE 0x00

bool loadBoundStateFromEEPROM();
void saveBoundStateToEEPROM(bool bound);
