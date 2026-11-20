# Mapa de Pinos — Placa A (Cérebro)
**Microcontrolador:** ESP32 WROOM-32D  
**Tensão:** 3.3V  
**Comunicação com Placa B:** UART2 (115200 8N1)

---

## Pinos Utilizados

| Função | GPIO | Tipo | Detalhes |
|--------|------|------|----------|
| **OLED SDA** | 21 | I2C | SSD1306 0x3C |
| **OLED SCL** | 22 | I2C | SSD1306 0x3C |
| **RTC SDA** | 21 | I2C | Compartilhado com OLED (DS3231 0x68) |
| **RTC SCL** | 22 | I2C | Compartilhado com OLED |
| **Teclado Linha 1** | 13 | Saída | Matriz 4×4 |
| **Teclado Linha 2** | 14 | Saída | Matriz 4×4 |
| **Teclado Linha 3** | 27 | Saída | Matriz 4×4 |
| **Teclado Linha 4** | 26 | Saída | Matriz 4×4 |
| **Teclado Coluna 1** | 25 | Entrada | INPUT_PULLUP |
| **Teclado Coluna 2** | 33 | Entrada | INPUT_PULLUP |
| **Teclado Coluna 3** | 32 | Entrada | INPUT_PULLUP |
| **Teclado Coluna 4** | 18 | Entrada | INPUT_PULLUP |
| **Buzzer Passivo** | 19 | Saída | PWM/LEDC |
| **Botão Segurar** | 23 | Entrada | INPUT_PULLUP (ativo LOW) |
| **Potenciômetro** | 34 | Entrada | ADC1 (apenas entrada) |
| **UART2 TX** | 17 | Saída | → RX Placa B |
| **UART2 RX** | 16 | Entrada | ← TX Placa B |

---

## Pinos Livres / Reservados
| GPIO | Status | Observação |
|------|--------|------------|
| 0 | Strapping | Boot mode (não usar) |
| 1 | TX0 | USB Serial (debug) |
| 2 | LED Built-in | LED interno azul |
| 3 | RX0 | USB Serial (debug) |
| 4 | Livre | Disponível |
| 5 | Livre | Disponível |
| 12 | Strapping | MTDI (não usar) |
| 15 | Strapping | MTDO (não usar) |
| 6-11 | Flash | SPI Flash (não usar) |
| 35-39 | Apenas entrada | Sem pull-up interno |

---

## Alimentação
| Pino | Função |
|------|--------|
| 5V (VIN) | Entrada 5V (USB ou bateria) → regula para 3.3V |
| 3.3V | Saída regulador interno (máx ~600mA) |
| GND | Terra comum |

> ⚠️ **Nota:** O potenciômetro usa GPIO 34 (ADC1), pois ADC2 não funciona com Wi-Fi ativo. O RTC e OLED compartilham barramento I2C (endereços 0x68 e 0x3C respectivamente).

---

## Ligação Placa A ↔ Placa B
| Placa A | Placa B | Cabo |
|---------|---------|-------|
| GPIO 17 (TX) | GPIO 16 (RX) | Cruzado |
| GPIO 16 (RX) | GPIO 17 (TX) | Cruzado |
| GND | GND | Comum |
| 5V (VIN) | 5V (VIN) | Comum |

> Alimentar **apenas pela Placa A** (USB ou bateria). Não conectar USB nas duas simultaneamente enquanto 5V estiver interligado.