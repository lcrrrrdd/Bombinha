#ifndef HAL_H
#define HAL_H

#include <Arduino.h>
#include "constants.h"

class HAL {
public:
  bool begin();

  // Fios (5)
  uint8_t readWires(); // bit 0=wire1, bit 4=wire5 (1=cortado/HIGH)

  // Chaves Toggle (4)
  uint8_t readToggles(); // bit 0=toggle1 (MSB), bit 3=toggle4

  // LEDs (8 total)
  void setLedMask(uint8_t mask); // bits 0-7 conforme LedIndex
  void updateLeds(); // PWM não-bloqueante para efeitos

  void setSimonLed(uint8_t idx, bool on); // 0-3
  void setStrikeLed(uint8_t idx, bool on); // 0-2
  void setSuccessLed(bool on);

  void reset(); // Apaga todos LEDs

private:
  // Pinos dos fios
  static constexpr uint8_t wirePins[5] = {
    PinB::WIRE_1, PinB::WIRE_2, PinB::WIRE_3, PinB::WIRE_4, PinB::WIRE_5
  };

  // Pinos dos toggles
  static constexpr uint8_t togglePins[4] = {
    PinB::TOGGLE_1, PinB::TOGGLE_2, PinB::TOGGLE_3, PinB::TOGGLE_4
  };

  // Pinos dos LEDs
  static constexpr uint8_t ledPins[8] = {
    PinB::LED_SIMON_A, PinB::LED_SIMON_B, PinB::LED_SIMON_C, PinB::LED_SIMON_D,
    PinB::LED_STRIKE_1, PinB::LED_STRIKE_2, PinB::LED_STRIKE_3,
    PinB::LED_SUCCESS
  };

  uint8_t _ledMask = 0;
  unsigned long _lastBlink = 0;
  bool _blinkState = false;
};

#endif // HAL_H