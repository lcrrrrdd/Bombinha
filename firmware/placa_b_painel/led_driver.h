#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <Arduino.h>
#include "../common/constants.h"

class LedDriver {
public:
  void begin();
  void setMask(uint8_t mask);
  void update();

private:
  static constexpr uint8_t ledPins[8] = {
    PinB::LED_SIMON_A,
    PinB::LED_SIMON_B,
    PinB::LED_SIMON_C,
    PinB::LED_SIMON_D,
    PinB::LED_STRIKE_1,
    PinB::LED_STRIKE_2,
    PinB::LED_STRIKE_3,
    PinB::LED_SUCCESS
  };

  uint8_t _currentMask = 0;
  uint8_t _targetMask = 0;
  unsigned long _blinkTimer = 0;
  bool _blinkState = false;
};

#endif // LED_DRIVER_H