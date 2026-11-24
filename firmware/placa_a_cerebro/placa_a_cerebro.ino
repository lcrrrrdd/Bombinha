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

#include "../common/constants.h"
#include "../common/protocol.h"
#include "hal.h"
#include "game_logic.h"
#include "wifi_ota.h"

// ------------------------------------------------------------
// Objetos Globais
// ------------------------------------------------------------
HAL hal;
GameLogic game;
WifiOTA ota;

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

  // OTA opcional
  ota.begin();
}

// ------------------------------------------------------------
// Loop Principal
// ------------------------------------------------------------
void loop() {
  // OTA opcional
  ota.handle();

  // Processa comunicação com Placa B
  handleUart();

  // Atualiza jogo (timer, strikes, máquina de estados)
  game.update();

  // Atualiza display
  updateDisplay();

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

// ------------------------------------------------------------
// Atualiza Display baseado no estado do jogo
// ------------------------------------------------------------
void updateDisplay() {
  hal.clearDisplay();

  // Timer (sempre visível)
  hal.drawTimer(game.getTimeRemaining());

  // Serial (sempre visível)
  hal.drawSerial(game.getSerial());

  // Nome da etapa
  const char* stageName = "";
  switch (game.getState()) {
    case GameState::STAGE_WIRE:   stageName = "CORTE O FIO"; break;
    case GameState::STAGE_PASS:   stageName = "DIGITE SENHA"; break;
    case GameState::STAGE_SIMON:  stageName = "SIMON SAYS"; break;
    case GameState::STAGE_FREQ:   stageName = "SINTONIZE FREQ"; break;
    case GameState::STAGE_TOGGLE: stageName = "AJUSTE CHAVES"; break;
    case GameState::STAGE_HOLD:   stageName = "SEGURE SOLTE"; break;
    default: break;
  }
  hal.drawStageName(stageName);

  // Strikes
  hal.drawStrikes(game.getStrikes());

  // Linha específica da etapa (linha 56)
  switch (game.getState()) {
    case GameState::STAGE_PASS:
      hal.drawPassword(game.getPassword(), game.getPasswordCursor());
      break;
    case GameState::STAGE_SIMON:
      if (game.isSimonShowing()) {
        hal.drawSimonSequence(game.getSimonSequence(), game.getSimonLen(), game.getSimonShowIdx());
      } else {
        // Durante input do jogador, mostra sequência parcial
        hal.drawSimonSequence(game.getSimonSequence(), game.getSimonLen(), game.getSimonUserStep());
      }
      break;
    case GameState::STAGE_FREQ:
      hal.drawFrequencyHint(game.getFreqDiff());
      break;
    case GameState::STAGE_TOGGLE:
      hal.drawToggles(game.getTargetToggles(), game.getCurrentToggles());
      break;
    case GameState::STAGE_HOLD:
      hal.drawHoldPrompt(game.getHoldTargetDigit());
      break;
    default:
      break;
  }

  hal.display();
}

// ------------------------------------------------------------
// RTC/NTP Test (opcional - pode ser removido)
// ------------------------------------------------------------
#ifdef RTC_NTP_TEST
void testRTC() {
  DateTime now = rtc.now();
  Serial.print("RTC: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
}
#endif