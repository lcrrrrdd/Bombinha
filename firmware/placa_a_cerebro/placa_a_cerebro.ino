/**
 * Placa A — Cérebro da Bomba
 * ESP32 WROOM-32D
 * 
 * Responsabilidades:
 * - Display OLED SSD1306 (I2C)
 * - Teclado Matricial 4x4
 * - Buzzer Passivo
 * - Botão Segurar/Soltar
 * - Potenciômetro
 * - RTC DS3231 (I2C)
 * - Lógica do jogo (máquina de estados)
 * - Comunicação UART com Placa B (Painel)
 * 
 * Pinos: veja firmware/common/constants.h
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <RTClib.h>

#include "constants.h"
#include "protocol.h"
#include "hal.h"
#include "game_logic.h"

// ------------------------------------------------------------
// Objetos Globais
// ------------------------------------------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;
Keypad keypad(makeKeymap(KEYPAD_MAP), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

HAL hal;
GameLogic game;

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup() {
  Serial.begin(BAUD_RATE);
  Serial2.begin(BAUD_RATE, SERIAL_8N1, PinA::UART_RX, PinA::UART_TX);

  // Inicializa HAL (pinos, display, RTC, etc.)
  if (!hal.begin()) {
    // Falha crítica - pisca LED interno
    pinMode(2, OUTPUT);
    while (true) {
      digitalWrite(2, HIGH); delay(100);
      digitalWrite(2, LOW);  delay(100);
    }
  }

  // Inicializa lógica do jogo
  game.begin(&hal, &Serial2);

  // Mostra tela inicial
  hal.showBootScreen();
  delay(2000);
  game.startNewGame();
}

// ------------------------------------------------------------
// Loop Principal
// ------------------------------------------------------------
void loop() {
  // Processa comunicação com Placa B
  handleUart();

  // Atualiza jogo (timer, strikes, máquina de estados)
  game.update();

  // Atualiza display
  hal.updateDisplay(game);

  // Pequeno delay para estabilidade
  delay(10);
}

// ------------------------------------------------------------
// Tratamento UART com Placa B
// ------------------------------------------------------------
void handleUart() {
  static String buffer;

  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      processEvent(buffer);
      buffer = "";
    } else {
      buffer += c;
    }
  }
}

// ------------------------------------------------------------
// Processa eventos vindos da Placa B
// ------------------------------------------------------------
void processEvent(const String& line) {
  if (line.length() == 0) return;

  char evtType;
  uint8_t value;
  if (!parseEvent(line, evtType, value)) return;

  switch (evtType) {
    case 'W': // Fio cortado
      game.onWireCut(value);
      break;
    case 'T': // Chaves toggle
      game.onTogglesChanged(value);
      break;
    case 'O': // OK/Heartbeat
      // Ignora
      break;
  }
}