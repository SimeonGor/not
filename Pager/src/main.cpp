#include <Arduino.h>

#include "business/pager_app.h"

namespace {
PagerApp app;
}  // namespace

void setup() {
  app.begin();
}

void loop() {
  app.loop();
}
