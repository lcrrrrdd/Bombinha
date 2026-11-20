#include "hal.h"

// ------------------------------------------------------------
// Inicialização
// ------------------------------------------------------------
bool HAL::begin() {
  // I2C
  Wire.begin(PinA::OLED_SDA, PinA::OLED_SCL);

  // Display
  _display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
  if (!_display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return false;
  }
  _display->clearDisplay();
  _display->setTextColor(WHITE);
  _display->setTextWrap(false);

  // RTC
  _rtc = new RTC_DS3231();
  if (!_rtc->begin()) {
    return false;
  }
  if (_rtc->lostPower()) {
    _rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Teclado
  static const byte rowPins[4] = {PinA::KEYPAD_R1, PinA::KEYPAD_R2, PinA::KEYPAD_R3, PinA::KEYPAD_R4};
  static const byte colPins[4] = {PinA::KEYPAD_C1, PinA::KEYPAD_C2, PinA::KEYPAD_C3, PinA::KEYPAD_C4};
  _keypad = new Keypad(makeKeymap(KEYPAD_MAP), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
  _keypad->setDebounceTime(20);

  // Pinos
  pinMode(PinA::BUZZER, OUTPUT);
  pinMode(PinA::BTN_HOLD, INPUT_PULLUP);
  pinMode(PinA::POT, INPUT);
  analogReadResolution(12); // 0-4095

  // UART2 para Placa B
  Serial2.begin(BAUD_RATE, SERIAL_8N1, PinA::UART_RX, PinA::UART_TX);

  return true;
}

// ------------------------------------------------------------
// Display
// ------------------------------------------------------------
void HAL::showBootScreen() {
  _display->clearDisplay();
  _display->setTextSize(2);
  _display->setCursor(10, 20);
  _display->print("BOMBINHA");
  _display->setTextSize(1);
  _display->setCursor(20, 45);
  _display->print("Inicializando...");
  _display->display();
}

void HAL::clearDisplay() {
  _display->clearDisplay();
}

void HAL::drawText(int x, int y, const char* text, uint8_t size) {
  _display->setTextSize(size);
  _display->setCursor(x, y);
  _display->print(text);
}

void HAL::drawCentered(int y, const char* text, uint8_t size) {
  _display->setTextSize(size);
  int16_t x1, y1;
  uint16_t w, h;
  _display->getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  _display->setCursor((SCREEN_WIDTH - w) / 2, y);
  _display->print(text);
}

void HAL::drawTimer(unsigned long msRemaining) {
  unsigned long totalSec = msRemaining / 1000;
  uint16_t mins = totalSec / 60;
  uint8_t secs = totalSec % 60;
  char buf[16];
  sprintf(buf, "%02d:%02d", mins, secs);
  _display->setTextSize(2);
  _display->setCursor(30, 0);
  _display->print(buf);
}

void HAL::drawSerial(const char* serial) {
  _display->setTextSize(1);
  _display->setCursor(0, 20);
  _display->print("SERIAL: ");
  _display->print(serial);
}

void HAL::drawStageName(const char* name) {
  _display->setTextSize(1);
  _display->setCursor(0, 32);
  _display->print("ETAPA: ");
  _display->print(name);
}

void HAL::drawStrikes(uint8_t strikes) {
  _display->setTextSize(1);
  _display->setCursor(0, 44);
  _display->print("STRIKES: ");
  _display->print(strikes);
  _display->print("/3");
}

void HAL::drawPassword(const char* password, uint8_t cursorPos) {
  _display->setTextSize(1);
  _display->setCursor(0, 56);
  _display->print("SENHA: ");
  for (uint8_t i = 0; i < 6; ++i) {
    if (i < cursorPos) {
      _display->print(password[i]);
    } else if (i == cursorPos) {
      _display->print("_");
    } else {
      _display->print("-");
    }
  }
}

void HAL::drawFrequencyHint(int16_t diff) {
  _display->setTextSize(1);
  _display->setCursor(0, 56);
  if (diff < -20) {
    _display->print("FREQ: MUITO BAIXA");
  } else if (diff < -5) {
    _display->print("FREQ: BAIXA");
  } else if (diff > 20) {
    _display->print("FREQ: MUITO ALTA");
  } else if (diff > 5) {
    _display->print("FREQ: ALTA");
  } else {
    _display->print("FREQ: SINTONIZADA!");
  }
}

void HAL::drawSimonSequence(const uint8_t* seq, uint8_t len, uint8_t current) {
  _display->clearDisplay();
  _display->setTextSize(1);
  _display->setCursor(0, 0);
  _display->print("SIMON SAYS");
  _display->setCursor(0, 16);
  for (uint8_t i = 0; i < len; ++i) {
    char c = "ABCD"[seq[i]];
    if (i < current) {
      _display->print(c);
      _display->print(" ");
    } else if (i == current) {
      _display->print("[");
      _display->print(c);
      _display->print("] ");
    } else {
      _display->print("- ");
    }
  }
  _display->display();
}

void HAL::drawToggles(uint8_t target, uint8_t current) {
  _display->setTextSize(1);
  _display->setCursor(0, 56);
  _display->print("TOGGLES: ");
  for (int i = 3; i >= 0; --i) {
    uint8_t tbit = (target >> i) & 1;
    uint8_t cbit = (current >> i) & 1;
    if (cbit == tbit) {
      _display->print(cbit);
    } else {
      _display->print("_");
    }
    _display->print(" ");
  }
}

void HAL::drawHoldPrompt(uint8_t targetDigit) {
  _display->setTextSize(1);
  _display->setCursor(0, 56);
  _display->print("SEGURE E SOLTE EM: ");
  _display->print(targetDigit);
}

void HAL::drawWin() {
  _display->clearDisplay();
  _display->setTextSize(2);
  drawCentered(20, "DESARMADA!");
  _display->setTextSize(1);
  drawCentered(48, "Parabens!");
  _display->display();
}

void HAL::drawLose() {
  _display->clearDisplay();
  _display->setTextSize(2);
  drawCentered(20, "BOOM!");
  _display->setTextSize(1);
  drawCentered(48, "Tente novamente");
  _display->display();
}

void HAL::display() {
  _display->display();
}

// ------------------------------------------------------------
// Teclado
// ------------------------------------------------------------
char HAL::getKey() {
  return _keypad->getKey();
}

bool HAL::waitForKey(char& key, unsigned long timeout) {
  unsigned long start = millis();
  while (timeout == 0 || millis() - start < timeout) {
    key = _keypad->getKey();
    if (key) return true;
    delay(10);
  }
  return false;
}

// ------------------------------------------------------------
// Buzzer
// ------------------------------------------------------------
void HAL::tone(uint16_t freq, uint16_t duration) {
  ::tone(PinA::BUZZER, freq, duration);
}

void HAL::noTone() {
  ::noTone(PinA::BUZZER);
}

void HAL::playMelody(const uint16_t* freqs, const uint16_t* durations, uint8_t len) {
  for (uint8_t i = 0; i < len; ++i) {
    tone(freqs[i], durations[i]);
    delay(durations[i] + 20);
  }
  noTone();
}

void HAL::playWinMelody() {
  constexpr uint16_t freqs[] = {523, 659, 784, 1047};
  constexpr uint16_t durs[]  = {150, 150, 150, 300};
  playMelody(freqs, durs, 4);
}

void HAL::playLoseAlarm() {
  for (int i = 0; i < 10; ++i) {
    tone(800, 100);
    delay(150);
    tone(400, 100);
    delay(150);
  }
  noTone();
}

void HAL::playStrikeBeep() {
  tone(200, 500);
  delay(500);
  noTone();
}

void HAL::playTick(bool fast) {
  tone(fast ? 1200 : 800, fast ? 50 : 100);
  delay(fast ? 80 : 150);
  noTone();
}

// ------------------------------------------------------------
// Botão Segurar
// ------------------------------------------------------------
bool HAL::isButtonPressed() {
  return digitalRead(PinA::BTN_HOLD) == LOW; // Pull-up: LOW = pressionado
}

bool HAL::waitForButtonRelease(uint8_t targetDigit, unsigned long& elapsedMs) {
  unsigned long start = millis();
  bool wasPressed = true;
  
  while (true) {
    bool pressed = isButtonPressed();
    
    if (wasPressed && !pressed) { // Soltou
      elapsedMs = millis() - start;
      return true;
    }
    wasPressed = pressed;
    
    // Timeout de segurança (30s)
    if (millis() - start > 30000) return false;
    
    delay(10);
  }
}

// ------------------------------------------------------------
// Potenciômetro
// ------------------------------------------------------------
int HAL::readPot() {
  return analogRead(PinA::POT); // 0-4095
}

int HAL::readPotMapped(int minOut, int maxOut) {
  int raw = readPot();
  return map(raw, 0, 4095, minOut, maxOut);
}

// ------------------------------------------------------------
// RTC
// ------------------------------------------------------------
bool HAL::rtcBegin() {
  return _rtc->begin();
}

DateTime HAL::rtcNow() {
  return _rtc->now();
}

void HAL::rtcAdjust(const DateTime& dt) {
  _rtc->adjust(dt);
}

// ------------------------------------------------------------
// Comunicação com Placa B
// ------------------------------------------------------------
void HAL::sendToPanel(const char* cmd) {
  Serial2.println(cmd);
}

void HAL::sendLedMask(uint8_t mask) {
  Serial2.print(CMD_LED_MASK);
  for (int i = 7; i >= 0; --i) {
    Serial2.print((mask >> i) & 1);
  }
  Serial2.println();
}

void HAL::requestPanelState() {
  Serial2.println(CMD_GET_STATE);
}

// ------------------------------------------------------------
// Debug
// ------------------------------------------------------------
void HAL::debugPrint(const char* msg) {
  Serial.print(msg);
}

void HAL::debugPrintln(const char* msg) {
  Serial.println(msg);
}