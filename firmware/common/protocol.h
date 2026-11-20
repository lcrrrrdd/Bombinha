#ifndef PROTOCOL_H
#define PROTOCOL_H

/**
 * Protocolo de comunicação UART entre Placa A (Cérebro) e Placa B (Painel)
 * Baudrate: 115200, 8N1
 * Formato: Comando ASCII terminado em '\n'
 */

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
// bit 0 = chave 1 (MSB ou LSB conforme implementação)
#define EVT_TOGGLES       "T"

// Heartbeat / alive (opcional)
#define EVT_ALIVE         "OK"

// ------------------------------------------------------------
// Funções utilitárias
// ------------------------------------------------------------

// Envia comando com máscara de 8 bits
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
// Retorna true se reconheceu e preenche 'value'
bool parseEvent(const String& line, char& evtType, uint8_t& value);

#endif // PROTOCOL_H