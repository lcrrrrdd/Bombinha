/**
 * Placa B — Painel da Bomba
 * ESP32 WROOM-32D
 * 
 * Responsabilidades:
 * - 5 Fios de corte (INPUT_PULLUP)
 * - 4 Chaves Toggle (INPUT com pull-up externo 10k)
 * - 8 LEDs (4 Simon + 3 Strike + 1 Sucesso)
 * - Comunicação UART com Placa A (Cérebro)
 * 
 * Pinos: veja firmware/common/constants.h
 */

#include <Arduino.h>
#include "constants.h"
#include "protocol.h"
#include "hal.h"

HAL hal;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial2.begin(BAUD_RATE, SERIAL_8N1, PinB::UART_RX, PinB::UART_TX);

  if (!hal.begin()) {
    // Falha crítica
    while (true) { delay(1000); }
  }
}

void loop() {
  // Lê fios (corte = HIGH = fio removido)
  static uint8_t lastWireState = 0;
  uint8_t wireState = hal.readWires();
  if (wireState != lastWireState) {
    // Detecta qual fio mudou de 0 para 1 (cortado)
    uint8_t changed = wireState & ~lastWireState;
    for (uint8_t i = 0; i < 5; ++i) {
      if (changed & (1 << i)) {
        Serial2.print(EVT_WIRE_CUT);
        Serial2.println(i + 1);
      }
    }
    lastWireState = wireState;
  }

  // Lê chaves toggle
  static uint8_t lastToggles = 0;
  uint8_t toggles = hal.readToggles();
  if (toggles != lastToggles) {
    Serial2.print(EVT_TOGGLES);
    // Envia como 4 bits binários (MSB first: toggle1..toggle4)
    for (int i = 3; i >= 0; --i) {
      Serial2.print((toggles >> i) & 1);
    }
    Serial2.println();
    lastToggles = toggles;
  }

  // Processa comandos da Placa A
  static String buffer;
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      processCommand(buffer);
      buffer = "";
    } else {
      buffer += c;
    }
  }

  // Atualiza LEDs (não bloqueante)
  hal.updateLeds();

  delay(5); // Pequeno delay para estabilidade
}

void processCommand(const String& line) {
  if (line.length() == 0) return;

  if (line.startsWith(CMD_LED_MASK)) {
    // L:10110000
    uint8_t mask = 0;
    String bits = line.substring(strlen(CMD_LED_MASK));
    if (bits.length() == 8) {
      for (int i = 0; i < 8; ++i) {
        if (bits[i] == '1') mask |= (1 << (7 - i));
      }
      hal.setLedMask(mask);
    }
  } else if (line == CMD_RESET) {
    hal.reset();
  }
}