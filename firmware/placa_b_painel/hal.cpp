#include "hal.h"

// Pinos (definidos no .h, replicados aqui para implementação)
constexpr uint8_t HAL::wirePins[5];
constexpr uint8_t HAL::togglePins[4];
constexpr uint8_t HAL::ledPins[8];

bool HAL::begin() {
  // Fios: INPUT_PULLUP (fio conectado = LOW, cortado = HIGH)
  for (uint8_t i = 0; i < 5; ++i) {
    pinMode(wirePins[i], INPUT_PULLUP);
  }

  // Toggles: INPUT (pull-up externo 10kΩ no hardware)
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(togglePins[i], INPUT);
  }

  // LEDs: OUTPUT
  for (uint8_t i = 0; i < 8; ++i) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  _ledMask = 0;
  return true;
}

uint8_t HAL::readWires() {
  uint8_t state = 0;
  for (uint8_t i = 0; i < 5; ++i) {
    // Fio cortado = HIGH (pull-up puxa para HIGH quando desconectado)
    if (digitalRead(wirePins[i]) == HIGH) {
      state |= (1 << i);
    }
  }
  return state;
}

uint8_t HAL::readToggles() {
  uint8_t state = 0;
  for (uint8_t i = 0; i < 4; ++i) {
    // Toggle ON = HIGH (pull-up externo, chave fecha para GND = LOW quando OFF)
    // Assumindo: chave na posição ON = HIGH (pull-up), OFF = LOW (GND)
    if (digitalRead(togglePins[i]) == HIGH) {
      // MSB first: toggle1 = bit 3, toggle4 = bit 0
      state |= (1 << (3 - i));
    }
  }
  return state;
}

void HAL::setLedMask(uint8_t mask) {
  _ledMask = mask;
  for (uint8_t i = 0; i < 8; ++i) {
    digitalWrite(ledPins[i], (mask >> i) & 1);
  }
}

void HAL::updateLeds() {
  // Para efeitos futuros (piscar, PWM, etc.)
  // Por enquanto, apenas mantém estado estático
}

void HAL::setSimonLed(uint8_t idx, bool on) {
  if (idx < 4) {
    if (on) _ledMask |= (1 << idx);
    else _ledMask &= ~(1 << idx);
    digitalWrite(ledPins[idx], on);
  }
}

void HAL::setStrikeLed(uint8_t idx, bool on) {
  if (idx < 3) {
    uint8_t bit = LED_STRIKE_1 + idx;
    if (on) _ledMask |= (1 << bit);
    else _ledMask &= ~(1 << bit);
    digitalWrite(ledPins[bit], on);
  }
}

void HAL::setSuccessLed(bool on) {
  if (on) _ledMask |= (1 << LED_SUCCESS);
  else _ledMask &= ~(1 << LED_SUCCESS);
  digitalWrite(ledPins[LED_SUCCESS], on);
}

void HAL::reset() {
  _ledMask = 0;
  for (uint8_t i = 0; i < 8; ++i) {
    digitalWrite(ledPins[i], LOW);
  }
}