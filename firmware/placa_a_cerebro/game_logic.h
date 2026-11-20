#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "hal.h"
#include "constants.h"
#include "protocol.h"

class GameLogic {
public:
  void begin(HAL* hal, Stream* panelSerial);
  void startNewGame();
  void update();
  
  // Callbacks de eventos da Placa B
  void onWireCut(uint8_t wireNum);
  void onTogglesChanged(uint8_t togglesMask);

  // Getters para display
  GameState getState() const { return _state; }
  uint8_t getStrikes() const { return _strikes; }
  unsigned long getTimeRemaining() const;
  const char* getSerial() const { return _serial; }
  const char* getPassword() const { return _password; }
  uint8_t getPasswordCursor() const { return _passwordCursor; }
  uint8_t getTargetWire() const { return _targetWire; }
  uint8_t getSimonStep() const { return _simonStep; }
  uint8_t getSimonUserStep() const { return _simonUserStep; }
  int16_t getFreqDiff() const { return _freqDiff; }
  uint8_t getTargetToggles() const { return _targetToggles; }
  uint8_t getCurrentToggles() const { return _currentToggles; }
  uint8_t getHoldTargetDigit() const { return _holdTargetDigit; }
  bool isSimonPlaying() const { return _simonPlaying; }

private:
  HAL* _hal = nullptr;
  Stream* _panel = nullptr;

  // Estado do jogo
  GameState _state = GameState::MENU;
  uint8_t _strikes = 0;
  unsigned long _gameStartMs = 0;
  unsigned long _stageStartMs = 0;
  unsigned long _accelMs = 0;

  // Serial da bomba (4 chars)
  char _serial[SERIAL_LENGTH + 1] = {0};

  // Senha (6 dígitos)
  char _password[7] = {0};
  uint8_t _passwordCursor = 0;

  // Etapas
  uint8_t _targetWire = 0;
  uint8_t _cutWire = 0;

  // Simon
  uint8_t _simonSequence[16] = {0};
  uint8_t _simonLen = 0;
  uint8_t _simonStep = 0;
  uint8_t _simonUserStep = 0;
  bool _simonPlaying = false;
  bool _simonShowing = false;
  unsigned long _simonTimer = 0;
  uint8_t _simonShowIdx = 0;

  // Frequência
  int _targetFreq = 0;
  int16_t _freqDiff = 0;

  // Chaves Toggle
  uint8_t _targetToggles = 0;
  uint8_t _currentToggles = 0;

  // Segurar/Soltar
  uint8_t _holdTargetDigit = 0;

  // --------------------------------------------------------
  // Métodos privados
  // --------------------------------------------------------
  void generateSerial();
  void generatePassword();
  uint8_t calcTargetWire();
  uint8_t calcTargetToggles();
  uint8_t calcHoldTargetDigit();
  int calcTargetFreq();
  void generateSimonSequence();

  void nextStage();
  void addStrike();
  void checkTimeUp();

  // Handlers por etapa
  void handleWire();
  void handlePassword();
  void handleSimon();
  void handleFrequency();
  void handleToggles();
  void handleHold();

  void sendLedMask(uint8_t mask);
  void setSimonLeds(uint8_t mask);
  void setStrikeLeds(uint8_t count);
  void setSuccessLed(bool on);
};

#endif // GAME_LOGIC_H