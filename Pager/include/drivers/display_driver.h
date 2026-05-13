#pragma once

#include "pager_types.h"

class DisplayDriver {
 public:
  bool begin();
  void drawUI(const PagerViewModel &viewModel);

 private:
  void drawStatusBar_(const __FlashStringHelper *statusText);
  void drawBootScreen_();
  void drawConnectingScreen_();
  void drawPairingScreen_(const PagerViewModel &viewModel);
  void drawErrorScreen_();
  void drawIdleScreen_(const PagerViewModel &viewModel);
  void drawReadingScreen_(const PagerViewModel &viewModel);
};
