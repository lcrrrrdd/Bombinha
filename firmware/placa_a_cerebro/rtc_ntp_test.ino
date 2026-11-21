/*
 * RTC DS3231 + WiFi NTP Sync - Teste Standalone
 * Placa: ESP32 DevKit V1
 * Display: Serial Monitor (115200 baud)
 * I2C: GPIO 21 (SDA), GPIO 22 (SCL)
 *
 * Comandos Serial:
 *   N/n : Sincronizar RTC via WiFi/NTP
 *   R/r : Mostrar hora atual do RTC + Uptime
 *   H/h : Mostrar ajuda
 *   W/w : Scan redes WiFi
 */

#include <Wire.h>
#include <RTClib.h>
#include <WiFi.h>
#include <time.h>

// ============================================================
// CONFIGURACOES - PREENCHA AQUI
// ============================================================
const char* WIFI_SSID     = "SEU_SSID_AQUI";
const char* WIFI_PASSWORD = "SUA_SENHA_AQUI";

const char* NTP_SERVER     = "pool.ntp.org";
const long  GMT_OFFSET_SEC = -3 * 3600;   // UTC-3 (Brasilia)
const int   DAYLIGHT_OFFSET_SEC = 0;      // Sem horario de verao

#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// ============================================================
// OBJETOS GLOBAIS
// ============================================================
RTC_DS3231 rtc;
unsigned long bootTime = 0;

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("\n========================================");
  Serial.println("  TESTE RTC DS3231 + NTP SYNC");
  Serial.println("========================================");

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!rtc.begin()) {
    Serial.println("ERRO: RTC nao encontrado! Verifique I2C (GPIO 21/22).");
    while (true) delay(1000);
  }

  if (rtc.lostPower()) {
    Serial.println("AVISO: RTC sem bateria - ajustando para compiliacao");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else {
    Serial.println("OK: RTC com bateria");
  }

  printRTCTime();
  bootTime = millis();
  printHelp();
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  handleSerialCommands();

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    printRTCTime();
    lastPrint = millis();
  }

  delay(10);
}

// ============================================================
// COMANDOS SERIAL
// ============================================================
void handleSerialCommands() {
  if (!Serial.available()) return;

  char cmd = Serial.read();
  while (Serial.available()) Serial.read(); // limpa buffer

  switch (cmd) {
    case 'N':
    case 'n':
      Serial.println("\n[NTP] Iniciando sincronizacao...");
      if (syncRTCfromNTP()) {
        Serial.println("[OK] RTC atualizado com sucesso!");
      } else {
        Serial.println("[ERRO] Falha na sincronizacao NTP");
      }
      printRTCTime();
      break;

    case 'R':
    case 'r':
      printRTCTime();
      break;

    case 'H':
    case 'h':
      printHelp();
      break;

    case 'W':
    case 'w':
      testWiFiScan();
      break;

    default:
      break;
  }
}

// ============================================================
// SINCRONIZACAO NTP -> RTC
// ============================================================
bool syncRTCfromNTP() {
  Serial.println("\n[NTP] Iniciando sincronizacao...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[WiFi] Conectando");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERRO] Falha ao conectar WiFi (timeout 15s)");
    WiFi.mode(WIFI_OFF);
    return false;
  }

  Serial.printf("[WiFi] Conectado! IP: %s\n", WiFi.localIP().toString().c_str());

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  struct tm timeinfo;
  unsigned long startNTP = millis();
  while (!getLocalTime(&timeinfo) && millis() - startNTP < 10000) {
    delay(100);
  }

  if (!getLocalTime(&timeinfo)) {
    Serial.println("[ERRO] Timeout NTP (10s)");
    WiFi.mode(WIFI_OFF);
    return false;
  }

  DateTime ntpTime(
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon + 1,
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  );
  rtc.adjust(ntpTime);

  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.printf("[NTP] OK RTC sincronizado: %s\n", buf);

  WiFi.mode(WIFI_OFF);
  return true;
}

// ============================================================
// IMPRESSAO DE HORA / UPTIME
// ============================================================
void printRTCTime() {
  DateTime now = rtc.now();

  char buf[32];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());

  unsigned long upMs = millis() - bootTime;
  unsigned long s = upMs / 1000;
  unsigned long h = s / 3600;
  unsigned long m = (s % 3600) / 60;
  s = s % 60;

  Serial.printf("[RTC] %s  |  Uptime: %02lu:%02lu:%02lu\n", buf, h, m, s);
}

// ============================================================
// AJUDA / SCANNER WIFI
// ============================================================
void printHelp() {
  Serial.println("\n--- COMANDOS ---");
  Serial.println("  N/n  : Sincronizar RTC via NTP (WiFi)");
  Serial.println("  R/r  : Mostrar hora atual do RTC");
  Serial.println("  H/h  : Mostrar ajuda");
  Serial.println("  W/w  : Scan redes WiFi");
  Serial.println("-------------------------------");
}

void testWiFiScan() {
  Serial.println("\n[WiFi] Escaneando redes...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  Serial.printf("[WiFi] %d redes encontradas:\n", n);
  for (int i = 0; i < n; i++) {
    Serial.printf("  %d: %s (%ddBm) %s\n", i + 1,
                  WiFi.SSID(i).c_str(),
                  WiFi.RSSI(i),
                  WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "(aberta)" : "");
  }
  WiFi.mode(WIFI_OFF);
}