# Mapa de Pinos — Placa B (Painel)
**Microcontrolador:** ESP32 WROOM-32D  
**Tensão:** 3.3V  
**Comunicação com Placa A:** UART2 (115200 8N1)

---

## Pinos Utilizados

| Função | GPIO | Tipo | Detalhes |
|--------|------|------|----------|
| **Fio 1** | 13 | Entrada | INPUT_PULLUP (cortado = HIGH) |
| **Fio 2** | 14 | Entrada | INPUT_PULLUP (cortado = HIGH) |
| **Fio 3** | 27 | Entrada | INPUT_PULLUP (cortado = HIGH) |
| **Fio 4** | 26 | Entrada | INPUT_PULLUP (cortado = HIGH) |
| **Fio 5** | 25 | Entrada | INPUT_PULLUP (cortado = HIGH) |
| **LED Simon A (Vermelho)** | 4 | Saída | 220Ω → LED → GND |
| **LED Simon B (Verde)** | 5 | Saída | 220Ω → LED → GND |
| **LED Simon C (Amarelo)** | 18 | Saída | 220Ω → LED → GND |
| **LED Simon D (Laranja)** | 19 | Saída | 220Ω → LED → GND |
| **LED Strike 1** | 21 | Saída | 220Ω → LED vermelho → GND |
| **LED Strike 2** | 22 | Saída | 220Ω → LED vermelho → GND |
| **LED Strike 3** | 23 | Saída | 220Ω → LED vermelho → GND |
| **LED Sucesso (Verde)** | 32 | Saída | 220Ω → LED verde → GND |
| **Chave Toggle 1** | 34 | Entrada | Pull-up 10kΩ externo → 3.3V |
| **Chave Toggle 2** | 35 | Entrada | Pull-up 10kΩ externo → 3.3V |
| **Chave Toggle 3** | 36 | Entrada | Pull-up 10kΩ externo → 3.3V |
| **Chave Toggle 4** | 39 | Entrada | Pull-up 10kΩ externo → 3.3V |
| **UART2 TX** | 17 | Saída | → RX Placa A |
| **UART2 RX** | 16 | Entrada | ← TX Placa A |

---

## Observações Importantes

### Pinos 34, 35, 36, 39 (GPIOs apenas entrada)
- **Não têm pull-up interno** no ESP32
- **Obrigatório** resistor de 10kΩ externo entre o pino e 3.3V
- Chave toggle liga o pino ao GND quando na posição OFF/desligada
- Posição ON = HIGH (pull-up), OFF = LOW (GND)

### Fios (GPIOs 13, 14, 27, 26, 25)
- Usam `INPUT_PULLUP` interno
- Fio intacto = LOW (conectado ao GND via borne)
- Fio cortado = HIGH (pull-up puxa para 3.3V)
- Bornes de 2 pinos na PCB para fácil substituição

### LEDs (8 total)
| Índice | Função | GPIO | Cor | Resistor |
|--------|--------|------|-----|----------|
| 0 | Simon A | 4 | Vermelho | 220Ω |
| 1 | Simon B | 5 | Verde | 220Ω |
| 2 | Simon C | 18 | Amarelo | 220Ω |
| 3 | Simon D | 19 | Laranja | 220Ω |
| 4 | Strike 1 | 21 | Vermelho | 220Ω |
| 5 | Strike 2 | 22 | Vermelho | 220Ω |
| 6 | Strike 3 | 23 | Vermelho | 220Ω |
| 7 | Sucesso | 32 | Verde | 220Ω |

Corrente ~6mA por LED (seguro para GPIO do ESP32).

---

## Pinos Livres
| GPIO | Status |
|------|--------|
| 0, 2, 12, 15 | Strapping (boot) — não usar |
| 1, 3 | USB Serial |
| 6-11 | Flash SPI |
| 33 | Livre (expansão futura) |

---

## Alimentação
| Pino | Função |
|------|--------|
| 5V (VIN) | Recebe 5V da Placa A |
| 3.3V | Regulador interno do ESP32 |
| GND | Comum com Placa A |

> ⚠️ **Não conectar USB na Placa B** enquanto alimentada pela Placa A.

---

## Ligação Placa B ↔ Placa A
| Placa B | Placa A | Cabo |
|---------|---------|-------|
| GPIO 16 (RX) | GPIO 17 (TX) | Cruzado |
| GPIO 17 (TX) | GPIO 16 (RX) | Cruzado |
| GND | GND | Comum |
| 5V (VIN) | 5V (VIN) | Comum |

> A Placa B é **totalmente passiva** — apenas reporta estado dos fios/chaves e acende LEDs sob comando da Placa A.