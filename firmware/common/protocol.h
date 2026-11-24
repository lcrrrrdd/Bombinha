#ifndef PROTOCOL_H
#define PROTOCOL_H

/**
 * Protocolo de comunicação UART entre Placa A (Cérebro) e Placa B (Painel)
 * Baudrate: 115200, 8N1
 * Formato: Comando ASCII terminado em '\n'
 */

#include "constants.h"

// ------------------------------------------------------------
// Comandos da Placa A → Placa B
// ------------------------------------------------------------

// Máscara dos 8 LEDs (Simon 4 + Strike 3 + Sucesso 1)
// Bits: 0=Simon A(Vm), 1=Simon B(Vd), 2=Simon C(Am), 3=Simon D(Lj),
//       4=Strike1, 5=Strike2, 6=Strike3, 7=Sucesso
// Exemplo: "L:10110000" → acende Simon A, C, D
#define CMD_LED_MASK      "L:"

// Reset do painel (nova partida)
#define CMD_RESET         "RST"

// Solicita estado atual dos fios e chaves
#define CMD_GET_STATE     "GET"

// ------------------------------------------------------------
// Eventos da Placa B → Placa A
// ------------------------------------------------------------

// Fio cortado: "W:<numero>"
// numero = 1 a 5
#define EVT_WIRE_CUT      "W"

// Estado das 4 chaves toggle: "T:<bits>"
// bits = 4 bits (ex: "T:1010" = chave1=1, chave2=0, chave3=1, chave4=0)
// bit 3 = chave 1 (MSB), bit 0 = chave 4 (LSB)
#define EVT_TOGGLES       "T"

// Heartbeat / alive (opcional)
#define EVT_ALIVE         "OK"

// ------------------------------------------------------------
// Funções utilitárias
// ------------------------------------------------------------

// Envia comando com máscara de 8 bits (MSB first)
inline void sendLedMask(Stream& port, uint8_t mask) {
  port.print(CMD_LED_MASK);
  for (int i = 7; i >= 0; --i) {
    port.print((mask >> i) & 1);
  }
  port.println();
}

// Envia comando de reset
inline void sendReset(Stream& port) {
  port.println(CMD_RESET);
}

// Solicita estado
inline void sendGetState(Stream& port) {
  port.println(CMD_GET_STATE);
}

// Parse de evento recebido
// Retorna true se reconheceu e preenche 'evtType' e 'value'
inline bool parseEvent(const String& line, char& evtType, uint8_t& value) {
  if (line.length() < 2) return false;

  // Formato: "X:valor" ou "XX" (para W e T sem :)
  if (line[1] == ':') {
    evtType = line[0];
    String valStr = line.substring(2);
    if (valStr.length() == 0) return false;

    if (evtType == 'W') {
      // W:N onde N = 1..5
      value = valStr.toInt();
      return (value >= 1 && value <= 5);
    }
    else if (evtType == 'T') {
      // T:XXXX onde XXXX = 4 bits binários
      if (valStr.length() != 4) return false;
      value = 0;
      for (int i = 0; i < 4; ++i) {
        if (valStr[i] == '1') value |= (1 << (3 - i));
        else if (valStr[i] != '0') return false;
      }
      return true;
    }
    else if (evtType == 'O') {
      // OK/Heartbeat
      value = 0;
      return valStr == "K";
    }
  }
  else if (line.length() == 2) {
    // Formato curto: WN ou TX (menos comum, mas suportado)
    evtType = line[0];
    char c = line[1];
    if (evtType == 'W' && c >= '1' && c <= '5') {
      value = c - '0';
      return true;
    }
    if (evtType == 'T') {
      // Não suporta formato curto para T (precisa 4 bits)
      return false;
    }
  }

  return false;
}

#endif // PROTOCOL_H