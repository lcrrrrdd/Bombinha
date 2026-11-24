#include "hal.h"

// ------------------------------------------------------------
// Inicialização
// ------------------------------------------------------------
bool HAL::begin() {
  // Fios de corte (INPUT_PULLUP)
  // Conectados ao GND quando intactos (LOW), HIGH quando cortados
  pinMode(PinB::WIRE_1, INPUT_PULLUP);
  pinMode(PinB::WIRE_2, INPUT_PULLUP);
  pinMode(PinB::WIRE_3, INPUT_PULLUP);
  pinMode(PinB::WIRE_4, INPUT_PULLUP);
  pinMode(PinB::WIRE_5, INPUT_PULLUP);

  // Chaves toggle (INPUT com pull-up externo 10k)
  // GPIO 34, 35, 36, 39 são input-only
  pinMode(PinB::TOGGLE_1, INPUT);
  pinMode(PinB::TOGGLE_2, INPUT);
  pinMode(PinB::TOGGLE_3, INPUT);
  pinMode(PinB::TOGGLE_4, INPUT);

  // LEDs
  for (uint8_t i = 0; i < 8; ++i) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  return true;
}

// ------------------------------------------------------------
// Fios
// ------------------------------------------------------------
uint8_t HAL::readWires() {
  uint8_t state = 0;
  if (digitalRead(PinB::WIRE_1) == HIGH) state |= (1 << 0);
  if (digitalRead(PinB::WIRE_2) == HIGH) state |= (1 << 1);
  if (digitalRead(PinB::WIRE_3) == HIGH) state |= (1 << 2);
  if (digitalRead(PinB::WIRE_4) == HIGH) state |= (1 << 3);
  if (digitalRead(PinB::WIRE_5) == HIGH) state |= (1 << 4);
  return state;
}

// ------------------------------------------------------------
// Chaves Toggle
// ------------------------------------------------------------
uint8_t HAL::readToggles() {
  uint8_t state = 0;
  // bit 3 = chave 1 (MSB), bit 0 = chave 4 (LSB)
  if (digitalRead(PinB::TOGGLE_1) == HIGH) state |= (1 << 3);
  if (digitalRead(PinB::TOGGLE_2) == HIGH) state |= (1 << 2);
  if (digitalRead(PinB::TOGGLE_3) == HIGH) state |= (1 << 1);
  if (digitalRead(PinB::TOGGLE_4) == HIGH) state |= (1 << 0);
  return state;
}

// ------------------------------------------------------------
// LEDs
// ------------------------------------------------------------
void HAL::setLedMask(uint8_t mask) {
  _ledMask = mask;
  // Atualiza imediatamente LEDs que não piscam
  for (uint8_t i = 0; i < 8; ++i) {
    bool on = (mask >> i) & 1;
    // Strike LEDs piscam se _strikeLeds > 0
    if (i >= LED_STRIKE_1 && i <= LED_STRIKE_3) {
      // Será tratado em updateLeds()
      continue;
    }
    digitalWrite(ledPins[i], on ? HIGH : LOW);
  }
}

void HAL::updateLeds() {
  // Atualiza LEDs de strike (podem piscar)
  unsigned long now = millis();
  if (now - _ledBlinkTimer > 500) {
    _ledBlinkTimer = now;
    _ledBlinkState = !_ledBlinkState;
  }

  // Strike LEDs: piscam se _strikeLeds > 0
  for (uint8_t i = LED_STRIKE_1; i <= LED_STRIKE_3; ++i) {
    bool shouldBeOn = ((_ledMask >> i) & 1) || (_strikeLeds > (i - LED_STRIKE_1) && _ledBlinkState);
    digitalWrite(ledPins[i], shouldBeOn ? HIGH : LOW);
  }

  // Success LED e Simon LEDs já estão em _ledMask (atualizados em setLedMask)
}

void HAL::reset() {
  _ledMask = 0;
  _strikeLeds = 0;
  _ledBlinkState = false;
  for (uint8_t i = 0; i < 8; ++i) {
    digitalWrite(ledPins[i], LOW);
  }
}