# 🧩 Lista de Componentes — Bombinha

Lista completa de hardware para montagem do jogo de desarme de bomba (2 placas ESP32).

---

## 🧠 Microcontroladores

| Qtd | Componente | Observação |
|-----|------------|------------|
| 2 | ESP32 WROOM-32D DevKit V1 (30 pinos) | Placa A (Cérebro) e Placa B (Painel) |

---

## 🖥️ Display

| Qtd | Componente | Endereço I2C | Observação |
|-----|------------|--------------|------------|
| 1 | Display OLED SSD1306 128×64 | 0x3C | I2C (SDA 21, SCL 22) |

---

## ⌨️ Entrada — Teclado

| Qtd | Componente | Pinos (Placa A) | Observação |
|-----|------------|-----------------|------------|
| 1 | Teclado Matricial 4x4 | Linhas: 13,14,27,26 | Colunas: 25,33,32,18 |
|   |                    |                 | 16 teclas: 0-9, A-D, *, # |

---

## ⚡ Entrada — Botões e Chaves

| Qtd | Componente | Pino (Placa A/B) | Observação |
|-----|------------|------------------|------------|
| 1 | Botão push-button grande | GPIO 23 (Placa A) | INPUT_PULLUP, outro lado no GND |
| 1 | Potenciômetro 10k | GPIO 34 (Placa A) | Cursor no pino, pontas em 3.3V e GND |
| 4 | Chaves toggle SPDT | GPIO 34, 35, 36, 39 (Placa B) | Resistor 10kΩ externo para 3.3V (não têm pull-up interno) |

---

## 🔌 Entrada — Fios de Corte

| Qtd | Componente | Pinos (Placa B) | Observação |
|-----|------------|-----------------|------------|
| 5 | Fios de corte (jumper macho-fêmea ou borne) | GPIO 13, 14, 27, 26, 25 (Placa B) | INPUT_PULLUP. Fio ligado ao GND = intacto (LOW). Remover = cortado (HIGH). |

---

## 💡 Saída — LEDs

| Qtd | Componente | Cores | Pinos (Placa B) | Resistor |
|-----|------------|-------|-----------------|----------|
| 4 | LEDs Simon | Vermelho, Verde, Amarelo, Laranja | 4, 5, 18, 19 | 220 Ω cada |
| 3 | LEDs Strike | Vermelho | 21, 22, 23 | 220 Ω cada |
| 1 | LED Sucesso | Verde | 32 | 220 Ω |

---

## 🔊 Saída — Áudio

| Qtd | Componente | Pino (Placa A) | Observação |
|-----|------------|----------------|------------|
| 1 | Buzzer passivo | GPIO 19 | Permite tons diferentes via PWM/LEDC |

---

## ⏰ RTC (Relógio Real)

| Qtd | Componente | Endereço I2C | Observação |
|-----|------------|--------------|------------|
| 1 | Módulo RTC DS3231 | 0x68 | Com bateria CR2032 (mantém hora sem energia) |

---

## 🔌 Conectores e Bornes

| Qtd | Componente | Observação |
|-----|------------|------------|
| 1 | Conector JST-XH 8 pinos | Teclado (Placa A) |
| 1 | Conector JST-XH 10 pinos | LEDs (Placa B) |
| 5 | Bornes de 2 pinos | Fios de corte (reposição fácil) |
| 1 | Conector JST-XH 5 pinos | Chaves toggle (4 sinais + GND) |
| 1 | Conector JST-XH 4 pinos | OLED (VCC, GND, SDA, SCL) |

---

## 🔌 Comunicação Entre Placas

| Componente | Observação |
|------------|------------|
| 3 fios jumper | TX/RX cruzado + GND comum (UART2, 115200 baud) |
| Alimentação 5V | Apenas Placa A alimentada via USB; 5V levado à Placa B |

---

## ⚡ Alimentação

| Qtd | Componente | Observação |
|-----|------------|------------|
| 1 | Power bank USB | Alimentação da Placa A |
| 1 | Bateria 18650 + módulo TP4056 | Alternativa ao power bank |
| 1 | Capacitor eletrolítico 100µF–470µF | Perto da entrada de energia (evita reset) |
| 1 | Capacitor cerâmico 100nF | No cursor do potenciômetro |

---

## 🛠️ PCB e Montagem

| Qtd | Componente | Observação |
|-----|------------|------------|
| 1 | PCB 10×10 cm (2 camadas) | JLCPCB/PCBWay (faixa econômica) |
| 2 | Soquete para ESP32 (30 pinos) | Não soldar o microcontrolador diretamente |
| 4 | Parafusos M3 + porcas | Fixação da PCB na caixa |
| 1 | Caixa em MDF | Painel frontal com recortes + tampa traseira removível |
| Vários | Fios jumpers macho-macho / macho-fêmea | Protoboard e ligações internas |

---

## 📦 Resumo de Quantidades Totais

| Categoria | Quantidade |
|-----------|------------|
| Microcontroladores | 2 |
| Display OLED | 1 |
| Teclado 4×4 | 1 |
| Buzzer passivo | 1 |
| Potenciômetro 10k | 1 |
| Botão push-button | 1 |
| Chaves toggle | 4 |
| Fios de corte | 5 |
| LEDs (Simon) | 4 |
| LEDs (Strike) | 3 |
| LED Sucesso | 1 |
| Resistores 220 Ω | 8 |
| Resistores 10 kΩ | 4 |
| RTC DS3231 | 1 |
| Bateria CR2032 | 1 |
| Conectores JST-XH | Vários |
| Bornes 2 pinos | 5 |
| PCB 10×10 cm | 1 |
| Soquetes ESP32 | 2 |

---

## 🔗 Arquivos Relacionados no Repositório

| Arquivo | Descrição |
|---------|-----------|
| `firmware/placa_a_cerebro/` | Código da Placa A (Cérebro) |
| `firmware/placa_b_painel/` | Código da Placa B (Painel) |
| `hardware/pinos/mapa_pinos_placa_a.md` | Pinagem Placa A |
| `hardware/pinos/mapa_pinos_placa_b.md` | Pinagem Placa B |
| `hardware/pcb/kicad/` | Projeto KiCad (PCB 10×10 cm) |
| `docs/MANUAL_BOMBA.html` | Manual do jogo |
| `COMPONENTES.md` | Esta lista |

---

## 🛒 Onde Comprar (Sugestão)

- Componentes eletrônicos: Mercado Livre, Shopee, Eletrônica Emporio, FilipeFlop, Amazon
- PCB: JLCPCB, PCBWay, OurPCB
- MDF/caixa: Marcenaria local ou corte a laser