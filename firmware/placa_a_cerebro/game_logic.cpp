#include "game_logic.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ------------------------------------------------------------
// Inicialização
// ------------------------------------------------------------
void GameLogic::begin(HAL* hal, Stream* panelSerial) {
  _hal = hal;
  _panel = panelSerial;
  randomSeed(analogRead(PinA::POT) ^ millis());
}

void GameLogic::startNewGame() {
  _strikes = 0;
  _accelMs = 0;
  _gameStartMs = _hal->millis();
  _stageStartMs = _hal->millis();

  generateSerial();
  generatePassword();
  _targetWire = calcTargetWire();
  _targetToggles = calcTargetToggles();
  _holdTargetDigit = calcHoldTargetDigit();
  _targetFreq = calcTargetFreq();

  // Ordem aleatória das etapas (fixa por simplicidade, pode embaralhar)
  // 0=WIRE, 1=PASS, 2=SIMON, 3=FREQ, 4=TOGGLE, 5=HOLD
  _state = GameState::STAGE_WIRE;
  _stageStartMs = _hal->millis();

  // Reset variáveis de etapa
  _passwordCursor = 0;
  memset(_password, '-', 6);
  _password[6] = 0;
  _cutWire = 0;
  _simonLen = 0;
  _simonStep = 0;
  _simonUserStep = 0;
  _simonPlaying = false;
  _simonShowing = false;
  _currentToggles = 0;
  _freqDiff = 0;

  // Notifica painel
  sendLedMask(0);
  _hal->sendToPanel(CMD_RESET);
}

// ------------------------------------------------------------
// Loop Principal
// ------------------------------------------------------------
void GameLogic::update() {
  checkTimeUp();

  switch (_state) {
    case GameState::STAGE_WIRE:   handleWire();   break;
    case GameState::STAGE_PASS:   handlePassword(); break;
    case GameState::STAGE_SIMON:  handleSimon();   break;
    case GameState::STAGE_FREQ:   handleFrequency(); break;
    case GameState::STAGE_TOGGLE: handleToggles(); break;
    case GameState::STAGE_HOLD:   handleHold();    break;
    case GameState::WIN:
      _hal->drawWin();
      break;
    case GameState::LOSE:
      _hal->drawLose();
      break;
    default: break;
  }
}

// ------------------------------------------------------------
// Geração do Serial (4 caracteres ASCII imprimíveis 33-126)
// ------------------------------------------------------------
void GameLogic::generateSerial() {
  for (uint8_t i = 0; i < SERIAL_LENGTH; ++i) {
    _serial[i] = char(random(33, 127)); // '!' (33) a '~' (126)
  }
  _serial[SERIAL_LENGTH] = 0;
}

// ------------------------------------------------------------
// Geração da Senha (6 dígitos)
// ------------------------------------------------------------
void GameLogic::generatePassword() {
  // D1: (ASCII(S[0]) % 9) + 1 → 1-9
  _password[0] = '0' + ((_serial[0] % 9) + 1);

  // D2: depende do fio cortado (preenchido depois em onWireCut)
  _password[1] = '-';

  // D3: (ASCII(S[2]) % 9) + 1 → 1-9
  _password[2] = '0' + ((_serial[2] % 9) + 1);

  // D4: (ASCII(S[1]) + strikes*17) % 10
  _password[3] = '0'; // será atualizado em updateStrikes()

  // D5: paridade de ASCII(S[2]) → par=5, ímpar=3
  _password[4] = (_serial[2] % 2 == 0) ? '5' : '3';

  // D6: depende do Simon (preenchido depois)
  _password[5] = '-';

  _password[6] = 0;
}

void GameLogic::updatePasswordD4() {
  _password[3] = '0' + ((_serial[1] + _strikes * 17) % 10);
}

// ------------------------------------------------------------
// Cálculos de Alvos
// ------------------------------------------------------------
uint8_t GameLogic::calcTargetWire() {
  // S[3] determina o fio:
  uint8_t c = _serial[3];
  
  if (isdigit(c)) return 5;           // dígito → fio 5 (Laranja)
  else if (isupper(c)) return 3;      // maiúscula → fio 3 (Verde)
  else if (islower(c)) return 4;      // minúscula → fio 4 (Amarelo)
  else if (c % 2 == 0) return 1;      // ASCII par → fio 1 (Vermelho)
  else return 2;                       // ASCII ímpar → fio 2 (Azul)
}

int GameLogic::calcTargetFreq() {
  // 440 + (S[0]%12)*50 + (S[2]%8)*10
  return 440 + (_serial[0] % 12) * 50 + (_serial[2] % 8) * 10;
}

uint8_t GameLogic::calcTargetToggles() {
  // Base: 4 LSBs de S[0]
  uint8_t val = _serial[0] & 0x0F;

  // Modificador fio Azul (fio 2) → XOR 0x0F
  if (_targetWire == 2) val ^= 0x0F;

  // Modificador Strikes
  if (_strikes > 0) val = (val + 1) & 0x0F;

  // Modificador S[3] primo → XOR 0x0A
  uint8_t s3 = _serial[3];
  if (isPrime(s3)) val ^= 0x0A;

  return val & 0x0F;
}

uint8_t GameLogic::calcHoldTargetDigit() {
  // D4 da senha
  return _password[3] - '0';
}

// ------------------------------------------------------------
// Senha
// ------------------------------------------------------------
void GameLogic::handlePassword() {
  char key = _hal->getKey();
  if (!key) return;

  if (key >= '0' && key <= '9') {
    if (_passwordCursor < 6) {
      _password[_passwordCursor] = key;
      _passwordCursor++;
    }
  } else if (key == '*') { // Backspace
    if (_passwordCursor > 0) {
      _passwordCursor--;
      // Não apaga D2, D4, D6 se já definidos - mantém placeholder
      if (_passwordCursor == 1) _password[1] = '-';
      else if (_passwordCursor == 3) { updatePasswordD4(); }
      else if (_passwordCursor == 5) _password[5] = '-';
      else _password[_passwordCursor] = '-';
    }
  } else if (key == '#') { // Confirmar
    if (_passwordCursor == 6) {
      if (strcmp(_password, "------") != 0) { // senha completa
        checkPassword();
      }
    }
  }
}

void GameLogic::checkPassword() {
  // Recalcula D4 com strikes atuais
  updatePasswordD4();
  
  // Gera senha correta completa
  char correct[7];
  correct[0] = _password[0]; // D1 já correto
  correct[1] = _password[1]; // D2 (definido pelo fio)
  correct[2] = _password[2]; // D3 já correto
  correct[3] = _password[3]; // D4 (com strikes)
  correct[4] = _password[4]; // D5 já correto
  correct[5] = _password[5]; // D6 (definido pelo Simon)
  correct[6] = 0;

  if (strcmp(_password, correct) == 0) {
    _hal->tone(1000, 200);
    nextStage();
  } else {
    addStrike();
    _passwordCursor = 0;
    memset(_password, '-', 6);
    generatePassword(); // regenera D1, D3, D5 (D2, D4, D6 mantêm lógica)
  }
}

// ------------------------------------------------------------
// Fios
// ------------------------------------------------------------
void GameLogic::handleWire() {
  // Aguarda evento da Placa B (processado via onWireCut)
}

void GameLogic::onWireCut(uint8_t wireNum) {
  if (_state != GameState::STAGE_WIRE) return;

  _cutWire = wireNum;

  if (wireNum == _targetWire) {
    _hal->tone(1000, 200);
    // Define D2 baseado na cor do fio
    switch (wireNum) {
      case 1: _password[1] = '3'; break; // Vermelho
      case 2: _password[1] = '8'; break; // Azul
      case 3: _password[1] = '2'; break; // Verde
      case 4: _password[1] = '7'; break; // Amarelo
      case 5: _password[1] = '4'; break; // Laranja
    }
    nextStage();
  } else {
    addStrike();
  }
}

// ------------------------------------------------------------
// Simon Says
// ------------------------------------------------------------
void GameLogic::generateSimonSequence() {
  // Seed = ASCII(S[1]) XOR ASCII(S[3])
  uint8_t seed = _serial[1] ^ _serial[3];
  randomSeed(seed);

  _simonLen = 5; // 5 cores para completar
  for (uint8_t i = 0; i < _simonLen; ++i) {
    _simonSequence[i] = random(0, 4); // 0=A, 1=B, 2=C, 3=D
  }
}

void GameLogic::handleSimon() {
  if (!_simonPlaying) {
    // Inicia Simon
    generateSimonSequence();
    _simonStep = 0;
    _simonUserStep = 0;
    _simonPlaying = true;
    _simonShowing = true;
    _simonShowIdx = 0;
    _simonTimer = _hal->millis();
    return;
  }

  if (_simonShowing) {
    // Mostra sequência
    unsigned long now = _hal->millis();
    if (now - _simonTimer > 600) {
      if (_simonShowIdx < _simonStep + 1) {
        // Mostra próxima cor
        uint8_t color = _simonSequence[_simonShowIdx];
        setSimonLeds(1 << color);
        _hal->tone(SIMON_FREQ[color], 300);
        _simonShowIdx++;
        _simonTimer = now;
      } else {
        // Fim da exibição
        setSimonLeds(0);
        _simonShowing = false;
        _simonUserStep = 0;
        _simonTimer = _hal->millis();
      }
    }
    return;
  }

  // Jogador repete
  char key = _hal->getKey();
  if (key >= 'A' && key <= 'D') {
    uint8_t pressed = key - 'A'; // A=0, B=1, C=2, D=3
    uint8_t expected = _simonSequence[_simonUserStep];

    if (pressed == expected) {
      _hal->tone(SIMON_FREQ[pressed], 150);
      _simonUserStep++;
      if (_simonUserStep >= _simonLen) {
        // Completou!
        _password[5] = '1'; // D6 = sucesso
        _hal->tone(1500, 300);
        delay(300);
        nextStage();
      }
    } else {
      // Errou
      addStrike();
      _password[5] = '0'; // D6 = falha
      _simonPlaying = false; // Reinicia Simon na próxima vez
    }
  }
}

// ------------------------------------------------------------
// Frequência
// ------------------------------------------------------------
void GameLogic::handleFrequency() {
  int potVal = _hal->readPotMapped(0, 2000); // 0-2000 Hz
  _freqDiff = potVal - _targetFreq;

  // Feedback sonoro: mais perto = mais agudo/rápido
  if (abs(_freqDiff) < 10) {
    _hal->playTick(true);
  } else if (abs(_freqDiff) < 30) {
    _hal->playTick(false);
  }

  // Confirmação: pressiona # quando sintonia
  char key = _hal->getKey();
  if (key == '#' && abs(_freqDiff) <= 5) {
    _hal->tone(1000, 200);
    nextStage();
  }
}

// ------------------------------------------------------------
// Chaves Toggle
// ------------------------------------------------------------
void GameLogic::handleToggles() {
  // Atualiza display com target vs current
  // Aguarda evento da Placa B (onTogglesChanged)
}

void GameLogic::onTogglesChanged(uint8_t togglesMask) {
  _currentToggles = togglesMask & 0x0F;

  if (_state == GameState::STAGE_TOGGLE && _currentToggles == _targetToggles) {
    _hal->tone(1000, 200);
    nextStage();
  }
}

// ------------------------------------------------------------
// Segurar e Soltar
// ------------------------------------------------------------
void GameLogic::handleHold() {
  if (!_hal->isButtonPressed()) return;

  unsigned long elapsed;
  if (_hal->waitForButtonRelease(_holdTargetDigit, elapsed)) {
    unsigned long totalSec = (_hal->millis() - _gameStartMs) / 1000;
    uint8_t lastDigit = totalSec % 10;
    
    if (lastDigit == _holdTargetDigit) {
      _hal->tone(1000, 200);
      nextStage();
    } else {
      addStrike();
    }
  }
}

// ------------------------------------------------------------
// Utilitários de Etapa
// ------------------------------------------------------------
void GameLogic::nextStage() {
  _hal->tone(1500, 200);
  
  switch (_state) {
    case GameState::STAGE_WIRE:   _state = GameState::STAGE_PASS;   break;
    case GameState::STAGE_PASS:   _state = GameState::STAGE_SIMON;  break;
    case GameState::STAGE_SIMON:  _state = GameState::STAGE_FREQ;   break;
    case GameState::STAGE_FREQ:   _state = GameState::STAGE_TOGGLE; break;
    case GameState::STAGE_TOGGLE: _state = GameState::STAGE_HOLD;   break;
    case GameState::STAGE_HOLD:   _state = GameState::WIN;          break;
    default: break;
  }
  _stageStartMs = _hal->millis();
  _simonPlaying = false;
}

void GameLogic::addStrike() {
  _strikes++;
  _accelMs += STRIKE_ACCEL_MS;
  _hal->playStrikeBeep();
  updatePasswordD4(); // D4 muda com strikes

  if (_strikes >= MAX_STRIKES) {
    _state = GameState::LOSE;
  }
}

void GameLogic::checkTimeUp() {
  unsigned long elapsed = _hal->millis() - _gameStartMs + _accelMs;
  if (elapsed >= GAME_DURATION_MS) {
    _state = GameState::LOSE;
  }
}

// ------------------------------------------------------------
// Getters
// ------------------------------------------------------------
unsigned long GameLogic::getTimeRemaining() const {
  unsigned long elapsed = _hal->millis() - _gameStartMs + _accelMs;
  if (elapsed >= GAME_DURATION_MS) return 0;
  return GAME_DURATION_MS - elapsed;
}

// ------------------------------------------------------------
// Comunicação com Painel
// ------------------------------------------------------------
void GameLogic::sendLedMask(uint8_t mask) {
  if (_panel) {
    _panel->print(CMD_LED_MASK);
    for (int i = 7; i >= 0; --i) {
      _panel->print((mask >> i) & 1);
    }
    _panel->println();
  }
}

void GameLogic::setSimonLeds(uint8_t mask) {
  uint8_t ledMask = 0;
  if (mask & 0x01) ledMask |= (1 << LED_SIMON_A);
  if (mask & 0x02) ledMask |= (1 << LED_SIMON_B);
  if (mask & 0x04) ledMask |= (1 << LED_SIMON_C);
  if (mask & 0x08) ledMask |= (1 << LED_SIMON_D);
  sendLedMask(ledMask);
}

void GameLogic::setStrikeLeds(uint8_t count) {
  uint8_t mask = 0;
  for (uint8_t i = 0; i < count && i < 3; ++i) {
    mask |= (1 << (LED_STRIKE_1 + i));
  }
  sendLedMask(mask);
}

void GameLogic::setSuccessLed(bool on) {
  uint8_t mask = on ? (1 << LED_SUCCESS) : 0;
  sendLedMask(mask);
}