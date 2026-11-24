#include "led_driver.h"

void LedDriver::begin() {
  for (uint8_t i = 0; i < 8; ++i) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
}

void LedDriver::setMask(uint8_t mask) {
  _targetMask = mask;
}

void LedDriver::update() {
  // Atualização imediata para LEDs não-piscantes
  uint8_t newMask = _targetMask;

  // Strike LEDs piscam se ativos
  unsigned long now = millis();
  if (now - _blinkTimer > 500) {
    _blinkTimer = now;
    _blinkState = !_blinkState;
  }

  for (uint8_t i = LED_STRIKE_1; i <= LED_STRIKE_3; ++i) {
    if ((_targetMask >> i) & 1) {
      if (_blinkState) newMask |= (1 << i);
      else newMask &= ~(1 << i);
    }
  }

  // Aplica apenas se mudou
  if (newMask != _currentMask) {
    for (uint8_t i = 0; i < 8; ++i) {
      bool on = (newMask >> i) & 1;
      digitalWrite(ledPins[i], on ? HIGH : LOW);
    }
    _currentMask = newMask;
  }
}