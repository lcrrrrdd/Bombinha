#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * Constantes globais do jogo — compartilhadas entre Placa A e B
 */

// ------------------------------------------------------------
// Tempo e Strikes
// ------------------------------------------------------------
constexpr unsigned long GAME_DURATION_MS     = 300000UL; // 5 minutos
constexpr unsigned long STRIKE_ACCEL_MS      = 30000UL;  // 30s a menos por strike
constexpr uint8_t         MAX_STRIKES        = 3;
constexpr unsigned long   WARNING_BEEP_MS    = 30000UL;  // últimos 30s

// ------------------------------------------------------------
// Estados do Jogo (máquina de estados)
// ------------------------------------------------------------
enum class GameState : uint8_t {
  MENU        = 0,
  STAGE_WIRE  = 1,   // Etapa 1: Fios
  STAGE_PASS  = 2,   // Etapa 2: Senha
  STAGE_SIMON = 3,   // Etapa 3: Simon Says
  STAGE_FREQ  = 4,   // Etapa 4: Frequência
  STAGE_TOGGLE= 5,   // Etapa 5: Chaves Toggle
  STAGE_HOLD  = 6,   // Etapa 6: Segurar/Soltar
  WIN         = 7,
  LOSE        = 8
};

// ------------------------------------------------------------
// Mapeamento dos 8 LEDs (Placa B)
// Bits na máscara L: (MSB first: bit7..bit0)
// ------------------------------------------------------------
enum LedIndex : uint8_t {
  LED_SIMON_A = 0,  // Vermelho
  LED_SIMON_B = 1,  // Verde
  LED_SIMON_C = 2,  // Amarelo
  LED_SIMON_D = 3,  // Azul/Laranja
  LED_STRIKE_1 = 4, // Strike 1
  LED_STRIKE_2 = 5, // Strike 2
  LED_STRIKE_3 = 6, // Strike 3
  LED_SUCCESS  = 7  // Verde Sucesso
};

// ------------------------------------------------------------
// Cores do Simon (para tons do buzzer)
// ------------------------------------------------------------
constexpr uint16_t SIMON_FREQ[4] = {
  262,  // A - Vermelho - Dó4
  330,  // B - Verde  - Mi4
  392,  // C - Amarelo - Sol4
  494   // D - Azul/Laranja - Si4
};

// ------------------------------------------------------------
// Pinos — Placa A (Cérebro) — ESP32 WROOM-32D
// ------------------------------------------------------------
namespace PinA {
  constexpr uint8_t OLED_SDA       = 21;
  constexpr uint8_t OLED_SCL       = 22;
  constexpr uint8_t KEYPAD_R1      = 13;
  constexpr uint8_t KEYPAD_R2      = 14;
  constexpr uint8_t KEYPAD_R3      = 27;
  constexpr uint8_t KEYPAD_R4      = 26;
  constexpr uint8_t KEYPAD_C1      = 25;
  constexpr uint8_t KEYPAD_C2      = 33;
  constexpr uint8_t KEYPAD_C3      = 32;
  constexpr uint8_t KEYPAD_C4      = 18;
  constexpr uint8_t BUZZER         = 19;
  constexpr uint8_t BTN_HOLD       = 23;
  constexpr uint8_t POT            = 34;
  constexpr uint8_t UART_TX        = 17;
  constexpr uint8_t UART_RX        = 16;
}

// ------------------------------------------------------------
// Pinos — Placa B (Painel) — ESP32 WROOM-32D
// ------------------------------------------------------------
namespace PinB {
  constexpr uint8_t WIRE_1         = 13;
  constexpr uint8_t WIRE_2         = 14;
  constexpr uint8_t WIRE_3         = 27;
  constexpr uint8_t WIRE_4         = 26;
  constexpr uint8_t WIRE_5         = 25;

  constexpr uint8_t LED_SIMON_A    = 4;
  constexpr uint8_t LED_SIMON_B    = 5;
  constexpr uint8_t LED_SIMON_C    = 18;
  constexpr uint8_t LED_SIMON_D    = 19;
  constexpr uint8_t LED_STRIKE_1   = 21;
  constexpr uint8_t LED_STRIKE_2   = 22;
  constexpr uint8_t LED_STRIKE_3   = 23;
  constexpr uint8_t LED_SUCCESS    = 32;

  constexpr uint8_t TOGGLE_1       = 34;  // precisa pull-up 10k externo
  constexpr uint8_t TOGGLE_2       = 35;  // precisa pull-up 10k externo
  constexpr uint8_t TOGGLE_3       = 36;  // precisa pull-up 10k externo
  constexpr uint8_t TOGGLE_4       = 39;  // precisa pull-up 10k externo

  constexpr uint8_t UART_TX        = 17;
  constexpr uint8_t UART_RX        = 16;
}

// ------------------------------------------------------------
// Display OLED
// ------------------------------------------------------------
constexpr uint8_t SCREEN_WIDTH  = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_ADDR     = 0x3C;

// ------------------------------------------------------------
// Serial / Comunicação
// ------------------------------------------------------------
constexpr uint32_t BAUD_RATE       = 115200;
constexpr uint32_t SERIAL_TIMEOUT  = 50; // ms

// ------------------------------------------------------------
// Teclado 4x4
// ------------------------------------------------------------
constexpr uint8_t KEYPAD_ROWS = 4;
constexpr uint8_t KEYPAD_COLS = 4;
constexpr char KEYPAD_MAP[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// ------------------------------------------------------------
// Serial da Bomba (4 caracteres ASCII imprimíveis)
// ------------------------------------------------------------
constexpr uint8_t SERIAL_LENGTH = 4;

// ------------------------------------------------------------
// Utilitários
// ------------------------------------------------------------
inline uint8_t asciiToNibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

inline bool isPrime(uint8_t n) {
  if (n < 2) return false;
  for (uint8_t i = 2; i * i <= n; ++i) if (n % i == 0) return false;
  return true;
}

#endif // CONSTANTS_H