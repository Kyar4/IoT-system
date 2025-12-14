#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "DHT.h"

#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

uint8_t MASTER_MAC[] = {0x8C, 0x4F, 0x00, 0x29, 0x7A, 0xB0};

static const uint8_t ESPNOW_CHANNEL = 1;
static const uint32_t SEND_INTERVAL_MS = 2000;

typedef struct __attribute__((packed)) {
  float t;
  float h;
  uint32_t ms;
  uint8_t node_id;
} EnvPacket;

EnvPacket pkt;
uint32_t lastSend = 0;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  (void)tx_info;
  Serial.print("[ESPNOW] Send: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}
#else
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  (void)mac_addr;
  Serial.print("[ESPNOW] Send: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}
#endif

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  Serial.print("[ESPNOW] RX from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X%s", info->src_addr[i], (i < 5) ? ":" : "");
  }
  Serial.print(" | len=");
  Serial.println(len);

  if (len >= 1) {
    uint8_t cmd = data[0];
    Serial.print("CMD=");
    Serial.println(cmd);
  }
}
#else
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
  Serial.print("[ESPNOW] RX from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X%s", mac[i], (i < 5) ? ":" : "");
  }
  Serial.print(" | len=");
  Serial.println(len);

  if (len >= 1) {
    uint8_t cmd = data[0];
    Serial.print("CMD=");
    Serial.println(cmd);
  }
}
#endif

void printMac(const uint8_t *mac) {
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X%s", mac[i], (i < 5) ? ":" : "");
  }
}

bool addPeer(const uint8_t *peerMac) {
  esp_now_peer_info_t peerInfo{};
  memcpy(peerInfo.peer_addr, peerMac, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_is_peer_exist(peerMac)) return true;

  esp_err_t r = esp_now_add_peer(&peerInfo);
  if (r != ESP_OK) {
    Serial.print("[ESPNOW] add_peer failed, err=");
    Serial.println((int)r);
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("SLAVE MAC: ");
  Serial.println(WiFi.macAddress());

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESPNOW] init failed");
    return;
  }

  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onRecv);

  Serial.print("MASTER MAC: ");
  printMac(MASTER_MAC);
  Serial.println();

  if (!addPeer(MASTER_MAC)) {
    Serial.println("[ESPNOW] Cannot add peer!");
    return;
  }

  dht.begin();

  pkt.node_id = 1;
  pkt.t = NAN;
  pkt.h = NAN;
  pkt.ms = millis();

  Serial.println("SLAVE ready.");
}

void loop() {
  uint32_t now = millis();
  if (now - lastSend < SEND_INTERVAL_MS) return;
  lastSend = now;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("[DHT] read failed (NaN), skip send");
    return;
  }

  pkt.h = h;
  pkt.t = t;
  pkt.ms = now;

  esp_err_t result = esp_now_send(MASTER_MAC, (uint8_t*)&pkt, sizeof(pkt));
  if (result != ESP_OK) {
    Serial.print("[ESPNOW] send error=");
    Serial.println((int)result);
  } else {
    Serial.printf("[SEND] T=%.1fC  H=%.1f%%  ms=%lu\n", pkt.t, pkt.h, (unsigned long)pkt.ms);
  }
}