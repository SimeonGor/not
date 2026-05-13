#include "services/eeprom_binding_store.h"

#include <EEPROM.h>

bool loadBoundStateFromEEPROM() {
  if (EEPROM.read(EEPROM_MAGIC_ADDR) != EEPROM_MAGIC_VALUE) {
    Serial.println(F("[EEPROM] no magic — treat bound cache as false"));
    return false;
  }
  const uint8_t v = EEPROM.read(EEPROM_BOUND_ADDR);
  const bool bound = (v == EEPROM_BOUND_TRUE);
  Serial.print(F("[EEPROM] loaded bound cache: "));
  Serial.println(bound ? F("true") : F("false"));
  return bound;
}

void saveBoundStateToEEPROM(const bool bound) {
  const uint8_t newByte = bound ? EEPROM_BOUND_TRUE : EEPROM_BOUND_FALSE;

  if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC_VALUE &&
      EEPROM.read(EEPROM_BOUND_ADDR) == newByte) {
    return;
  }

  EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
  EEPROM.write(EEPROM_BOUND_ADDR, newByte);
  const bool ok = EEPROM.commit();
  Serial.print(F("[EEPROM] saved bound="));
  Serial.print(bound ? F("true") : F("false"));
  Serial.print(F(" commit="));
  Serial.println(ok ? F("ok") : F("fail"));
}
