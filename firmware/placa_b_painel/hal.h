#ifndef HAL_B_H
#define HAL_B_H

#include <Arduino.h>
#include "../common/constants.h"

class HAL {
public:
  // Inicialização
  bool begin();

  // Fios de corte (5 fios, INPUT_PULLUP)
  // Retorna bitmask: bit 0 = fio 1, bit 4 = fio 5
  // 0 = intacto (conectado ao GND), 1 = cortado (HIGH)
  uint8_t readWires();

  // Chaves toggle (4 chaves, pull-up externo 10k)
  // Retorna bitmask: bit 3 = chave 1, bit 0 = chave 4
  uint8_t readToggles();

  // LEDs
  void setLedMask(uint8_t mask);
  void updateLeds();
  void reset();

private:
  // Estados dos LEDs para atualização não bloqueante
  uint8_t _ledMask = 0;
  unsigned long _ledBlinkTimer = 0;
  bool _ledBlinkState = false;
  uint8_t _strikeLeds = 0;

  // Pinos dos LEDs
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
};

#endif // HAL_B_H