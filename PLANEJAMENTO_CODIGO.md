# PLANEJAMENTO TÉCNICO — BOMBINHA (Firmware ESP32)

Documento único consolidando: arquitetura, máquina de estados, protocolo UART, telas OLED, lógica de jogo, dependências cruzadas e validações.

---

## 1. ARQUITETURA GERAL

### 1.1 Hardware — Duas Placas ESP32 WROOM-32D

| Placa | Papel | Principais Periféricos |
|-------|-------|------------------------|
| **Placa A (Cérebro)** | Mestre — lógica do jogo, display, teclado, RTC | OLED SSD1306 I2C, Teclado 4×4, Buzzer passivo, Botão hold, Potenciômetro, RTC DS3231, UART2 → Placa B |
| **Placa B (Painel)** | Escrava — I/O físico do painel | 5 fios (INPUT_PULLUP), 4 chaves toggle (pull-up 10k externo), 8 LEDs (4 Simon + 3 Strike + 1 Sucesso), UART2 → Placa A |

### 1.2 Comunicação UART

- **Baudrate:** 115200, 8N1
- **Conexão:** TX↔RX cruzado + GND comum + 5V da A → VIN da B
- **Protocolo:** Comandos ASCII terminados em `\n`

**Placa A → Placa B (Comandos):**
```
L:XXXXXXXX   // Máscara 8 bits dos LEDs (bit7=Sucesso, bit6=Strike3, bit5=Strike2, bit4=Strike1, bit3=SimonD, bit2=SimonC, bit1=SimonB, bit0=SimonA)
RST          // Reset do painel (nova partida)
GET          // Solicita estado dos fios e chaves
```

**Placa B → Placa A (Eventos):**
```
W:N          // Fio N cortado (1-5)
T:XXXX       // Estado das 4 chaves (4 bits, MSB=chave1)
OK           // Heartbeat/alive
```

### 1.3 Estrutura de Pastas do Firmware

```
firmware/
├── common/
│   ├── constants.h    # Pinos, estados, constantes, utilitários
│   └── protocol.h     # Definições e helpers do protocolo UART
├── placa_a_cerebro/
│   ├── placa_a_cerebro.ino    // Setup/loop principal, tratamento UART
│   ├── hal.h / hal.cpp        # Hardware Abstraction Layer (OLED, RTC, teclado, buzzer, botão, pot)
│   ├── game_logic.h / .cpp    # Máquina de estados, 6 etapas, geração de alvos
│   └── wifi_ota.h / .cpp      # OTA opcional
└── placa_b_painel/
    ├── placa_b_painel.ino     // Loop: lê fios/chaves, processa comandos, atualiza LEDs
    ├── hal.h / .cpp           # HAL do painel (fios, chaves, LEDs)
    └── led_driver.h / .cpp    # Driver dos 8 LEDs
```

---

## 2. MÁQUINA DE ESTADOS (GameState)

```cpp
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
```

**Fluxo fixo (não volta atrás):**
```
MENU → STAGE_WIRE → STAGE_PASS → STAGE_SIMON → STAGE_FREQ → STAGE_TOGGLE → STAGE_HOLD → WIN
                                    ↓ (strike ou timeout)
                                  LOSE
```

**Transições em `nextStage()`:**
- WIRE → PASS → SIMON → FREQ → TOGGLE → HOLD → WIN

---

## 3. CÓDIGO SECRETO (Serial) — Âncora Universal

- **Geração:** 4 caracteres ASCII imprimíveis aleatórios (33-126 = '!' a '~')
- **Função `generateSerial()`:** `random(33, 127)` para cada posição
- **Armazenamento:** `char _serial[5]` (4 chars + null terminator)

| Posição | Variável | Usado em |
|---------|----------|----------|
| 0 (1º)  | `_serial[0]` | D1, chave base, frequência base |
| 1 (2º)  | `_serial[1]` | D4, seed Simon |
| 2 (3º)  | `_serial[2]` | D3, D5, frequência extra |
| 3 (4º)  | `_serial[3]` | Fio alvo, D6 (via Simon), chave extra |

---

## 4. ETAPAS DETALHADAS — LÓGICA E DEPENDÊNCIAS

### 4.1 Etapa 1 — Fios (STAGE_WIRE)

**Alvo calculado em `calcTargetWire()` no `startNewGame()`:**
```cpp
uint8_t c = _serial[3];
if (isdigit(c)) return 5;           // dígito → fio 5 (Laranja)
else if (isupper(c)) return 3;      // maiúscula → fio 3 (Verde)
else if (islower(c)) return 4;      // minúscula → fio 4 (Amarelo)
else if (c % 2 == 0) return 1;      // ASCII par → fio 1 (Vermelho)
else return 2;                       // ASCII ímpar → fio 2 (Azul)
```

**Evento `onWireCut(wireNum)` vindo da Placa B:**
- Se `wireNum == _targetWire`: acerta → define D2, `nextStage()`
- Se errado: `addStrike()`, permanece na etapa

**Tabela D2 (2º dígito da senha) definida aqui:**
| Fio cortado | D2 |
|-------------|-----|
| 1 (Vermelho) | 3 |
| 2 (Azul)     | 8 |
| 3 (Verde)    | 2 |
| 4 (Amarelo)  | 7 |
| 5 (Laranja)  | 4 |

---

### 4.2 Etapa 2 — Senha (STAGE_PASS)

**Geração inicial em `generatePassword()` (no `startNewGame()`):**
```cpp
_password[0] = '0' + ((_serial[0] % 9) + 1);      // D1: 1-9
_password[1] = '-';                                // D2: preenchido no onWireCut
_password[2] = '0' + ((_serial[2] % 9) + 1);      // D3: 1-9
_password[3] = '0';                                // D4: atualizado via updatePasswordD4()
_password[4] = (_serial[2] % 2 == 0) ? '5' : '3'; // D5: par=5, ímpar=3
_password[5] = '-';                                // D6: preenchido no Simon
```

**Atualização D4 (chamada a cada strike e antes de validar):**
```cpp
_password[3] = '0' + ((_serial[1] + _strikes * 17) % 10);
```

**Validação em `checkPassword()` (tecla # com 6 dígitos):**
1. Recalcula D4 com strikes atuais
2. Gera senha correta completa (D1-D6 já definidos)
3. Compara `_password` com `correct`
4. Se igual: `nextStage()`
5. Se diferente: `addStrike()`, reseta cursor, regenera D1/D3/D5 (mantém D2/D4/D6)

**Entrada do teclado:**
- 0-9: preenche `_password[_passwordCursor++]`
- *: backspace (não apaga D2/D4/D6 se já definidos — volta para placeholder '-')
- #: confirma se cursor == 6

---

### 4.3 Etapa 3 — Simon Says (STAGE_SIMON)

**Semente:** `seed = _serial[1] ^ _serial[3]` (XOR do 2º e 4º caractere)

**Sequência:** 5 cores fixas (`_simonLen = 5`), geradas via `random(0,4)` após `randomSeed(seed)`

**Mapeamento cores → teclas → frequências:**
| Índice | Cor | Tecla | Frequência (Hz) |
|--------|-----|-------|-----------------|
| 0 | Vermelho | A | 262 (Dó4) |
| 1 | Verde | B | 330 (Mi4) |
| 2 | Amarelo | C | 392 (Sol4) |
| 3 | Azul/Laranja | D | 494 (Si4) |

**Fluxo interno:**
1. `generateSimonSequence()` — gera array `_simonSequence[5]`
2. **Fase exibição (`_simonShowing=true`):** mostra uma cor a cada 600ms (LED + tom)
3. **Fase jogador:** lê teclas A-D, compara com `_simonSequence[_simonUserStep]`
4. Acertou: incrementa `_simonUserStep`; se chegou em 5 → `_password[5]='1'`, `nextStage()`
5. Errou: `addStrike()`, `_password[5]='0'`, `_simonPlaying=false` (reinicia na próxima entrada)

**Importante:** O 6º dígito (D6) é definido **aqui** — por isso a Etapa 2 deixa placeholder.

---

### 4.4 Etapa 4 — Frequência (STAGE_FREQ)

**Alvo calculado em `calcTargetFreq()` no `startNewGame()`:**
```cpp
_targetFreq = 440 + (_serial[0] % 12) * 50 + (_serial[2] % 8) * 10;
```
Range: 440 Hz a ~1000 Hz

**Loop `handleFrequency()`:**
- Lê potenciômetro mapeado 0-2000 Hz (`readPotMapped(0, 2000)`)
- `_freqDiff = potVal - _targetFreq`
- Feedback sonoro:
  - `|diff| < 10`: tick rápido (sintonizado)
  - `|diff| < 30`: tick lento (perto)
  - Senão: silencioso
- Confirmação: tecla `#` com `|diff| <= 5` → `nextStage()`

**Dica no OLED (via `drawFrequencyHint`):**
| Diff | Texto |
|------|-------|
| < -20 | "FREQ: MUITO BAIXA" |
| -20 a -5 | "FREQ: BAIXA" |
| -5 a 5 | "FREQ: SINTONIZADA!" |
| 5 a 20 | "FREQ: ALTA" |
| > 20 | "FREQ: MUITO ALTA" |

---

### 4.5 Etapa 5 — Chaves Toggle (STAGE_TOGGLE)

**Alvo calculado em `calcTargetToggles()` no `startNewGame()`:**
```cpp
uint8_t val = _serial[0] & 0x0F;        // Base: 4 LSBs do 1º caractere
if (_targetWire == 2) val ^= 0x0F;      // Se fio Azul cortado → inverte todos
if (_strikes > 0) val = (val + 1) & 0x0F; // Se strikes > 0 → +1 com carry
if (isPrime(_serial[3])) val ^= 0x0A;   // Se 4º caractere primo → XOR 0x0A (bits 1 e 3)
return val & 0x0F;
```

**Evento `onTogglesChanged(mask)` vindo da Placa B:**
- Atualiza `_currentToggles`
- Se `_state == STAGE_TOGGLE` e `mask == _targetToggles` → `nextStage()`

**Display no OLED (`drawToggles`):** mostra alvo vs atual bit a bit (match = bit, mismatch = `_`)

---

### 4.6 Etapa 6 — Segurar e Soltar (STAGE_HOLD)

**Alvo calculado em `calcHoldTargetDigit()` no `startNewGame()`:**
```cpp
return _password[3] - '0';  // D4 da senha
```

**Loop `handleHold()`:**
- Aguarda botão pressionado (`isButtonPressed()` = LOW)
- `waitForButtonRelease(targetDigit, elapsed)` — retorna quando solta
- No momento do release: lê `totalSec % 10` do timer
- Se `lastDigit == _holdTargetDigit` → `nextStage()` → WIN
- Senão → `addStrike()`

---

## 5. SISTEMA DE STRIKES E TEMPO

### 5.1 Strikes
- **Máximo:** 3 (`MAX_STRIKES = 3`)
- **Cada strike:** `_strikes++`, `_accelMs += 30000` (30 segundos de aceleração)
- **Efeitos colaterais:**
  - Atualiza D4 da senha (`updatePasswordD4()`)
  - Afeta cálculo das chaves toggle (se strikes > 0, +1)
  - Acende LEDs Strike 1/2/3 na Placa B
  - Se `_strikes >= 3` → `STATE = LOSE`

### 5.2 Timer
- **Duração base:** 300.000 ms (5 minutos)
- **Tempo decorrido:** `millis() - _gameStartMs + _accelMs`
- **Aviso crítico:** últimos 30s (`WARNING_BEEP_MS = 30000`) → buzzer mais rápido
- **Timeout:** se `elapsed >= GAME_DURATION_MS` → `STATE = LOSE`

---

## 6. TELA OLED — DESIGN DAS TELAS (128×64)

### 6.1 Layout Base (todas as etapas exceto Simon)

```
┌────────────────────────────────────────┐
│  00:00                    (timer 2x)   │  Linha 0-15: timer grande
├────────────────────────────────────────┤
│  SERIAL: K9#m                          │  Linha 20: código secreto
├────────────────────────────────────────┤
│  ETAPA: CORTE O FIO                    │  Linha 32: nome da etapa
├────────────────────────────────────────┤
│  STRIKES: 0/3                          │  Linha 44: strikes
├────────────────────────────────────────┤
│  [Linha específica da etapa]           │  Linha 56: conteúdo dinâmico
└────────────────────────────────────────┘
```

### 6.2 Linha 56 por Etapa

| Etapa | Conteúdo da Linha 56 (`drawPassword` / `drawFrequencyHint` / `drawToggles` / `drawHoldPrompt`) |
|-------|------------------------------------------------------------------------------------------------|
| WIRE | (vazio — aguarda evento fio) |
| PASS | `SENHA: 1234_5` (cursor piscando, `-` = não digitado) |
| SIMON | Tela dedicada (ver 6.3) |
| FREQ | `FREQ: SINTONIZADA!` / `FREQ: ALTA` / etc. |
| TOGGLE | `TOGGLES: 1 0 _ 1` (match = bit, mismatch = `_`) |
| HOLD | `SEGURE E SOLTE EM: 7` |

### 6.3 Tela Simon (tela cheia, `drawSimonSequence`)

```
┌────────────────────────────────────────┐
│  SIMON SAYS                            │
├────────────────────────────────────────┤
│  A B C D                               │  sequência (len=5)
│  [B] - - - -                           │  atual em destaque
└────────────────────────────────────────┘
```
- Pisca LEDs na Placa B simultaneamente
- Tons no buzzer a cada 600ms

### 6.4 Telas Finais

**WIN (`drawWin`):**
```
      DESARMADA!
      
      Parabéns!
```

**LOSE (`drawLose`):**
```
      BOOM!
      
      Tente novamente
```

### 6.5 Boot Screen (`showBootScreen`)
```
   BOMBINHA
   
   Inicializando...
```

---

## 7. PROTOCOLO UART — IMPLEMENTAÇÃO DETALHADA

### 7.1 Placa A (Mestre) — Envio

```cpp
// Máscara LEDs (8 bits: bit7..bit0 = Sucesso, Strike3,2,1, SimonD,C,B,A)
void sendLedMask(uint8_t mask) {
  Serial2.print("L:");
  for (int i = 7; i >= 0; --i) Serial2.print((mask >> i) & 1);
  Serial2.println();
}

// Reset painel
void sendReset() { Serial2.println("RST"); }

// Solicita estado
void sendGetState() { Serial2.println("GET"); }
```

### 7.2 Placa A — Recepção (em `handleUart()` / `processEvent()`)

```cpp
// Buffer linha a linha até \n
// Parse via parseEvent(line, evtType, value)

switch (evtType) {
  case 'W': onWireCut(value); break;        // value = 1..5
  case 'T': onTogglesChanged(value); break; // value = 0..15 (4 bits)
  case 'O': break; // heartbeat
}
```

### 7.3 Placa B (Escrava) — Loop Principal

```cpp
void loop() {
  // 1. Lê fios (INPUT_PULLUP): intacto=LOW, cortado=HIGH
  //    Detecta transição 0→1 = fio cortado → envia "W:N"
  
  // 2. Lê chaves (4 inputs com pull-up externo)
  //    Se mudou → envia "T:XXXX" (4 bits MSB first)
  
  // 3. Processa comandos da Placa A (buffer até \n)
  //    "L:XXXXXXXX" → setLedMask(mask)
  //    "RST" → reset() (apaga LEDs, reseta estado)
  
  // 4. Atualiza LEDs (não bloqueante)
  hal.updateLeds();
  
  delay(5);
}
```

### 7.4 Mapeamento Bits LED (Placa B)

| Bit | LED | GPIO (Placa B) |
|-----|-----|----------------|
| 0 | Simon A (Vermelho) | 4 |
| 1 | Simon B (Verde) | 5 |
| 2 | Simon C (Amarelo) | 18 |
| 3 | Simon D (Azul/Laranja) | 19 |
| 4 | Strike 1 | 21 |
| 5 | Strike 2 | 22 |
| 6 | Strike 3 | 23 |
| 7 | Sucesso (Verde) | 32 |

---

## 8. MAPEAMENTO DE PINOS (constants.h)

### 8.1 Placa A — Cérebro

| Função | GPIO | Observação |
|--------|------|------------|
| OLED SDA | 21 | I2C compartilhado |
| OLED SCL | 22 | I2C compartilhado |
| Keypad R1-R4 | 13,14,27,26 | Linhas |
| Keypad C1-C4 | 25,33,32,18 | Colunas |
| Buzzer | 19 | PWM/LEDC |
| Botão Hold | 23 | INPUT_PULLUP |
| Potenciômetro | 34 | ADC1_CH6 (input only) |
| UART2 TX | 17 | → RX Placa B |
| UART2 RX | 16 | ← TX Placa B |

### 8.2 Placa B — Painel

| Função | GPIO | Observação |
|--------|------|------------|
| Fio 1-5 | 13,14,27,26,25 | INPUT_PULLUP |
| LED Simon A-D | 4,5,18,19 | 220Ω cada |
| LED Strike 1-3 | 21,22,23 | 220Ω cada |
| LED Sucesso | 32 | 220Ω |
| Toggle 1-4 | 34,35,36,39 | **Pull-up 10k externo obrigatório** |
| UART2 TX | 17 | → RX Placa A |
| UART2 RX | 16 | ← TX Placa A |

---

## 9. DEPENDÊNCIAS CRUZADAS — RESUMO PARA VALIDAÇÃO

| Origem | Afeta | Momento |
|--------|-------|---------|
| Serial[0] | D1, freq base, chave base | startNewGame |
| Serial[1] | D4, seed Simon | startNewGame / Simon |
| Serial[2] | D3, D5, freq extra | startNewGame |
| Serial[3] | Fio alvo, D6 (via Simon), chave extra | startNewGame / Simon / Toggle |
| Fio cortado (Et1) | D2, chave extra (se Azul) | onWireCut / calcTargetToggles |
| Strikes | D4, chave extra (+1) | addStrike / calcTargetToggles |
| Simon resultado | D6 | handleSimon |

**Ordem de execução garante que dependências estão prontas:**
1. `startNewGame()` calcula tudo que não depende de ações do jogador
2. Etapa 1 define D2 e `_targetWire`
3. Etapa 2 usa D2, strikes atuais, Serial[0..2] — D4 recalculado a cada strike
4. Etapa 3 define D6
5. Etapa 4 usa Serial[0], Serial[2]
6. Etapa 5 usa `_targetWire`, strikes atuais, Serial[0], Serial[3]
7. Etapa 6 usa D4 (já finalizado)

---

## 10. VALIDAÇÕES NECESSÁRIAS (Checklist)

### 10.1 Lógica de Jogo
- [ ] Serial gerado com 4 chars ASCII 33-126
- [ ] `calcTargetWire()` cobre todos os casos (dígito, maiúscula, minúscula, par, ímpar)
- [ ] D1/D3: `(val % 9) + 1` → 1-9
- [ ] D2: tabela fixa por cor do fio
- [ ] D4: `(Serial[1] + strikes*17) % 10` → 0-9, atualiza a cada strike
- [ ] D5: paridade de Serial[2] → 5 ou 3
- [ ] D6: 1 se Simon ok, 0 se errou
- [ ] Senha validada corretamente em `checkPassword()`
- [ ] Backspace (*) não apaga D2/D4/D6 definidos
- [ ] Simon: seed = Serial[1] ^ Serial[3], 5 cores, reinicia se errou
- [ ] Frequência: fórmula correta, tolerância ±5 Hz
- [ ] Chaves: base (Serial[0]&0x0F) → XOR 0x0F se fio Azul → +1 se strikes → XOR 0x0A se Serial[3] primo
- [ ] Hold: alvo = D4, solta no `totalSec % 10`
- [ ] Strikes: +30s cada, max 3 = LOSE
- [ ] Timer: 5 min base + aceleração, timeout = LOSE

### 10.2 UART/Comunicação
- [ ] Placa A envia `L:` máscara correta para cada evento
- [ ] Placa A envia `RST` no `startNewGame()`
- [ ] Placa B detecta fio cortado (0→1) e envia `W:N`
- [ ] Placa B detecta mudança chaves e envia `T:XXXX`
- [ ] Placa B processa `L:` e `RST` corretamente
- [ ] Timeout/buffer não trava comunicação

### 10.3 OLED/Display
- [ ] Boot screen aparece
- [ ] Timer conta regressivo corretamente (mm:ss)
- [ ] Serial exibido
- [ ] Nome da etapa correto
- [ ] Strikes: X/3
- [ ] Senha: mostra dígitos digitados, `_` no cursor, `-` nos vazios
- [ ] Dica frequência: 5 níveis corretos
- [ ] Simon: tela cheia com sequência
- [ ] Toggles: alvo vs atual bit a bit
- [ ] Hold: "SEGURE E SOLTE EM: N"
- [ ] WIN/LOSE telas finais

### 10.4 Hardware/Periféricos
- [ ] I2C OLED + RTC compartilhado funcionando
- [ ] Teclado 4×4 lendo todas as 16 teclas
- [ ] Buzzer: tons Simon, ticks frequência, melodias win/lose, strike beep
- [ ] Botão hold: INPUT_PULLUP, detecta press/release
- [ ] Potenciômetro: ADC 12-bit, mapeado 0-2000Hz
- [ ] RTC: mantém hora, battery backup
- [ ] Placa B: fios INPUT_PULLUP, chaves pull-up externo, LEDs 220Ω
- [ ] UART2: 115200, TX/RX cruzado, GND comum

---

## 11. BUILD E DEPLOY

### 11.1 Requisitos Arduino IDE
- ESP32 Board Manager: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Board: **ESP32 Dev Module**
- Flash: 4MB, Partition: "Default 4MB with spiffs"
- Bibliotecas: `Adafruit SSD1306`, `Adafruit GFX`, `Keypad`, `RTClib`

### 11.2 Ordem de Upload
1. `firmware/placa_a_cerebro/placa_a_cerebro.ino` → Placa A
2. `firmware/placa_b_painel/placa_b_painel.ino` → Placa B

### 11.3 Simulação Wokwi
- `simulacao/placa_a/diagram.json` — testa OLED, teclado, timer, lógica
- `simulacao/placa_b/diagram.json` — testa fios, chaves, LEDs
- **Limitação:** Wokwi não suporta multi-MCU simultâneo

---

## 12. ARQUIVOS DE REFERÊNCIA NO REPOSITÓRIO

| Arquivo | Descrição |
|---------|-----------|
| `firmware/common/constants.h` | Pinos, estados, constantes, utilitários |
| `firmware/common/protocol.h` | Protocolo UART |
| `firmware/placa_a_cerebro/game_logic.h/.cpp` | Lógica completa do jogo (máquina de estados) |
| `firmware/placa_a_cerebro/hal.h/.cpp` | HAL Placa A (OLED, RTC, teclado, buzzer, botão, pot) |
| `firmware/placa_b_painel/hal.h/.cpp` | HAL Placa B (fios, chaves, LEDs) |
| `firmware/placa_b_painel/led_driver.h/.cpp` | Driver LEDs Placa B |
| `hardware/pinos/mapa_pinos_placa_a.md` | Documentação pinos Placa A |
| `hardware/pinos/mapa_pinos_placa_b.md` | Documentação pinos Placa B |
| `simulacao/placa_a/diagram.json` | Wokwi Placa A |
| `simulacao/placa_b/diagram.json` | Wokwi Placa B |

---

*Documento gerado a partir da análise do firmware real (game_logic.cpp, hal.cpp, constants.h, protocol.h) do repositório lcrrrrdd/Bombinha.*
