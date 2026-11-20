#ifndef HAL_H
#define HAL_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <RTClib.h>
#include "constants.h"

class HAL {
public:
  // Inicialização
  bool begin();

  // Display
  void showBootScreen();
  void clearDisplay();
  void drawText(int x, int y, const char* text, uint8_t size = 1);
  void drawCentered(int y, const char* text, uint8_t size = 1);
  void drawTimer(unsigned long msRemaining);
  void drawSerial(const char* serial);
  void drawStageName(const char* name);
  void drawStrikes(uint8_t strikes);
  void drawPassword(const char* password, uint8_t cursorPos);
  void drawFrequencyHint(int16_t diff); // -100 a +100
  void drawSimonSequence(const uint8_t* seq, uint8_t len, uint8_t current);
  void drawToggles(uint8_t target, uint8_t current);
  void drawHoldPrompt(uint8_t targetDigit);
  void drawWin();
  void drawLose();
  void display();

  // Teclado
  char getKey();
  bool waitForKey(char& key, unsigned long timeout = 0);

  // Buzzer
  void tone(uint16_t freq, uint16_t duration);
  void noTone();
  void playMelody(const uint16_t* freqs, const uint16_t* durations, uint8_t len);
  void playWinMelody();
  void playLoseAlarm();
  void playStrikeBeep();
  void playTick(bool fast);

  // Botão Segurar
  bool isButtonPressed();
  bool waitForButtonRelease(uint8_t targetDigit, unsigned long& elapsedMs);

  // Potenciômetro
  int readPot();           // 0-4095
  int readPotMapped(int minOut, int maxOut);

  // RTC
  bool rtcBegin();
  DateTime rtcNow();
  void rtcAdjust(const DateTime& dt);

  // Serial/Comunicação
  void sendToPanel(const char* cmd);
  void sendLedMask(uint8_t mask);
  void requestPanelState();

  // Serial (debug)
  void debugPrint(const char* msg);
  void debugPrintln(const char* msg);

  // Utilitários
  unsigned long millis() { return ::millis(); }
  void delayMs(uint32_t ms) { ::delay(ms); }

private:
  Adafruit_SSD1306* _display = nullptr;
  RTC_DS3231* _rtc = nullptr;
  Keypad* _keypad = nullptr;

  // Pinos do teclado
  static constexpr byte rowPins[4] = {PinA::KEYPAD_R1, PinA::KEYPAD_R2, PinA::KEYPAD_R3, PinA::KEYPAD_R4};
  static constexpr byte colPins[4] = {PinA::KEYPAD_C1, PinA::KEYPAD_C2, PinA::KEYPAD_C3, PinA::KEYPAD_C4};
};

#endif // HAL_H