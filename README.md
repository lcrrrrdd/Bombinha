# 💣 Bombinha — Jogo de Desarme de Bomba

Projeto educativo de eletrônica: um simulador de desarme de bomba com 6 etapas dinâmicas, hardware baseado em **2× ESP32 WROOM-32D** (ou 2× Arduino Nano), display OLED SSD1306, teclado matricial 4×4, potenciômetro, chaves toggle, fios físicos, buzzer e LEDs.

---

## 🎯 Objetivo
Desarmar a bomba completando todas as 6 etapas antes que o timer de **5 minutos** chegue a zero. Máximo de **3 strikes** (erros). Cada strike acelera o timer.

---

## 📋 Etapas do Jogo

| Etapa | Descrição | Dependência do Serial |
|-------|-----------|----------------------|
| 1. **Fios** | Cortar o fio correto entre 5 opções | S[3] (4º caractere) |
| 2. **Senha** | 6 dígitos via teclado 4×4 | S[0], S[1], S[2] + fio cortado + strikes + Simon |
| 3. **Simon Says** | Repetir sequência de 4 LEDs/cores | Seed = S[1] XOR S[3] |
| 4. **Frequência** | Sintonizar potenciômetro até tom alvo | S[0] + S[2] |
| 5. **Chaves Toggle** | 4 chaves = 16 combinações binárias | S[0] + fio Azul + strikes + S[3] |
| 6. **Segurar/Soltar** | Soltar botão no dígito alvo | D4 da senha |

---

## 🔌 Hardware

### Placa A — Cérebro
- ESP32 WROOM-32D
- OLED SSD1306 I2C (GPIO 21/22)
- Teclado 4×4 (GPIOs 13,14,27,26 + 25,33,32,18)
- Buzzer passivo (GPIO 19)
- Botão segurar (GPIO 23)
- Potenciômetro 10k (GPIO 34)
- RTC DS3231 (I2C compartilhado)
- UART2 → Placa B (GPIO 16/17)

### Placa B — Painel
- ESP32 WROOM-32D
- 5 fios (GPIOs 13,14,27,26,25)
- 4 chaves toggle (GPIOs 34,35,36,39 + pull-ups 10kΩ)
- 8 LEDs: 4 Simon (4,5,18,19) + 3 Strike (21,22,23) + 1 Sucesso (32)
- UART2 → Placa A (GPIO 16/17)

### Comunicação
- UART a 115200 baud (TX↔RX cruzado)
- GND comum
- 5V da Placa A → VIN da Placa B

---

## 📁 Estrutura do Repositório

```
Bombinha/
├── .github/workflows/ci.yml      # CI opcional
├── .gitignore
├── LICENSE                       # MIT
├── README.md
├── docs/
│   └── MANUAL_BOMBA.html         # Manual com ícones SVG
├── firmware/
│   ├── common/                   # Compartilhado entre placas
│   │   ├── protocol.h            # Protocolo UART
│   │   └── constants.h           # Constantes do jogo
│   ├── placa_a_cerebro/          # Placa A (mestre)
│   │   ├── placa_a_cerebro.ino
│   │   ├── hal.h/.cpp            # Hardware Abstraction Layer
│   │   ├── game_logic.h/.cpp     # Lógica do jogo
│   │   └── wifi_ota.h/.cpp       # OTA opcional
│   └── placa_b_painel/           # Placa B (escrava)
│       ├── placa_b_painel.ino
│       ├── hal.h/.cpp
│       └── led_driver.h/.cpp
├── hardware/
│   ├── pcb/kicad/                # Projeto KiCad 10×10 cm
│   ├── esquematico/              # PDF do esquema
│   └── pinos/                    # Mapas de GPIO
│       ├── mapa_pinos_placa_a.md
│       └── mapa_pinos_placa_b.md
├── icons/                        # 10 ícones SVG originais
├── simulacao/
│   ├── placa_a/diagram.json      # Wokwi
│   └── placa_b/diagram.json
└── .github/workflows/ci.yml
```

---

## 🚀 Como Compilar e Carregar

### Requisitos
- Arduino IDE 2.x ou VS Code + PlatformIO
- ESP32 Board Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Bibliotecas:
  - `Adafruit SSD1306`
  - `Adafruit GFX Library`
  - `Keypad`
  - `RTClib`

### Passos
1. Abra `firmware/placa_a_cerebro/placa_a_cerebro.ino` na Arduino IDE
2. Selecione **ESP32 Dev Module** (ou sua placa)
3. Configure: Flash 4MB, Partition "Default 4MB with spiffs"
4. Carregue na **Placa A**
5. Repita para `firmware/placa_b_painel/placa_b_painel.ino` na **Placa B**

---

## 🧪 Simulação no Wokwi

1. Abra `simulacao/placa_a/diagram.json` no [Wokwi](https://wokwi.com)
2. Teste a lógica da Placa A (OLED, teclado, timer)
3. Repita para `simulacao/placa_b/diagram.json`
4. **Nota:** Wokwi não suporta multi-MCU simultâneo ainda. Teste uma placa por vez.

---

## 📦 PCB e Montagem

- **Tamanho:** 10×10 cm (faixa econômica JLCPCB/PCBWay)
- **Soquetes** para ESP32 (não soldar direto)
- **Conectores JST-XH** para teclado e LEDs
- **Bornes 2 pinos** para fios substituíveis
- **Furos M3** nos cantos para fixação na caixa MDF

Veja `hardware/pcb/kicad/` para arquivos do KiCad.

---

## 📖 Manual

Abra `docs/MANUAL_BOMBA.html` no navegador → **Ctrl+P → Salvar como PDF** para versão impressa.

---

## 📄 Licença

MIT License — veja [LICENSE](LICENSE).

---

## 🤝 Contribuições

Pull requests são bem-vindos! Para mudanças maiores, abra uma issue primeiro.