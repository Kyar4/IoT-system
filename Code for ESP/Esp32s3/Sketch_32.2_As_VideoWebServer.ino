#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "esp_camera.h"

#define CAMERA_MODEL_ESP32S3_EYE
#include "camera_pins.h"

// ===== WiFi list =====
struct WifiCred { const char* ssid; const char* pass; };
WifiCred WIFI_LIST[] = {
  {"NekryuHost",     "Nvt@3112004"},
  {"Xiaomi 14T pro", "23112004"},
  {"Bao Nghia",      "27132713"},
  {"Tường Vinh",     "hana4869"},
};
const int WIFI_COUNT = sizeof(WIFI_LIST) / sizeof(WIFI_LIST[0]);
int lastWifiIdx = 0;

// ===== Flask server =====
const char* serverUrl = "http://192.168.1.239:8000/upload";

// ===== MQTT HiveMQ Cloud (SỬA 3 dòng này) =====
const char* HIVEMQ_HOST = "3a73d99203e7455aa6e3f7e90aea1e6e.s1.eu.hivemq.cloud";  // <-- sửa
const int   HIVEMQ_PORT = 8883;
const char* HIVEMQ_USER = "Nakashima";             // <-- sửa
const char* HIVEMQ_PASS = "@Hana4869";             // <-- sửa

const char* TOPIC_PUB   = "esp32s3/face";              // topic publish kết quả
const char* TOPIC_LWT   = "esp32s3/face/status";       // online/offline

WiFiClientSecure tls;
PubSubClient mqtt(tls);

// ====== Camera init ======
void cameraInit() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;

  // Gợi ý nhẹ hơn để detect nhanh: QVGA
  config.frame_size   = FRAMESIZE_QVGA;     // VGA -> nặng, QVGA nhẹ
  config.pixel_format = PIXFORMAT_JPEG;

  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;                // số lớn hơn = nhẹ hơn (7 nét hơn nhưng nặng hơn)
  config.fb_count     = psramFound() ? 2 : 1;

  if (!psramFound()) {
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed 0x%x\n", err);
    while (true) delay(1000);
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_vflip(s, 0);
    s->set_hmirror(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, 0);
  }
}

// ====== WiFi connect ======
bool connectOne(int idx, uint32_t timeoutMs = 15000) {
  WiFi.disconnect(false);
  delay(100);
  WiFi.begin(WIFI_LIST[idx].ssid, WIFI_LIST[idx].pass);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    lastWifiIdx = idx;
    Serial.printf("\nWiFi OK: %s | IP: ", WIFI_LIST[idx].ssid);
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.printf("\nWiFi FAIL: %s\n", WIFI_LIST[idx].ssid);
  return false;
}

bool connectBestKnownWiFi(uint32_t timeoutMs = 15000) {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  int n = WiFi.scanNetworks(false, true);
  int bestIdx = -1, bestRssi = -999;

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      String found = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      for (int k = 0; k < WIFI_COUNT; k++) {
        if (found == WIFI_LIST[k].ssid && rssi > bestRssi) {
          bestRssi = rssi;
          bestIdx = k;
        }
      }
    }
  }
  WiFi.scanDelete();

  if (bestIdx != -1 && connectOne(bestIdx, timeoutMs)) return true;

  for (int i = 0; i < WIFI_COUNT; i++) {
    int idx = (lastWifiIdx + i) % WIFI_COUNT;
    if (connectOne(idx, timeoutMs)) return true;
  }
  return false;
}

void ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.println("\nWiFi lost -> reconnect...");
  while (WiFi.status() != WL_CONNECTED) {
    if (connectBestKnownWiFi(15000)) break;
    delay(3000);
  }
}

// ====== MQTT connect ======
void ensureMQTT() {
  if (mqtt.connected()) return;

  while (!mqtt.connected()) {
    String clientId = "ESP32S3-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("MQTT connect...");

    if (mqtt.connect(clientId.c_str(), HIVEMQ_USER, HIVEMQ_PASS, TOPIC_LWT, 1, true, "offline")) {
      Serial.println("OK");
      mqtt.publish(TOPIC_LWT, "online", true);
      return;
    }

    Serial.printf("FAIL rc=%d, retry...\n", mqtt.state());
    delay(2000);
  }
}

// ====== Parse JSON nhẹ (không ArduinoJson) ======
bool parseFaceAndCount(const String& body, bool &face, int &count) {
  face = false;
  count = 0;

  int pFace = body.indexOf("\"face\"");
  if (pFace >= 0) {
    int c = body.indexOf(':', pFace);
    if (c >= 0) {
      int i = c + 1;
      while (i < (int)body.length() && (body[i] == ' ' || body[i] == '\t')) i++;
      face = (i < (int)body.length() && body[i] == '1');
    }
  } else {
    // fallback nếu server trả "1"/"0"
    String b = body; b.trim();
    if (b == "1" || b == "true" || b == "True") face = true;
  }

  int pCount = body.indexOf("\"count\"");
  if (pCount >= 0) {
    int c = body.indexOf(':', pCount);
    if (c >= 0) {
      int i = c + 1;
      while (i < (int)body.length() && (body[i] == ' ' || body[i] == '\t')) i++;
      String num = "";
      while (i < (int)body.length() && isDigit(body[i])) { num += body[i]; i++; }
      if (num.length() > 0) count = num.toInt();
    }
  }

  return true;
}

// ====== Send frame -> Flask -> publish MQTT ======
void sendFrameToServerAndPublish() {
  if (WiFi.status() != WL_CONNECTED) return;

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  unsigned long t0 = millis();

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "image/jpeg");

  int code = http.POST(fb->buf, fb->len);
  String body = (code > 0) ? http.getString() : "";

  http.end();
  esp_camera_fb_return(fb);

  unsigned long dt = millis() - t0;

  if (code <= 0) {
    Serial.printf("HTTP POST error: %d\n", code);
    return;
  }

  bool face = false;
  int count = 0;
  parseFaceAndCount(body, face, count);

  Serial.printf("HTTP %d | face=%d | count=%d | %lums\n", code, face ? 1 : 0, count, dt);

  // Publish MQTT (retain để app mở lên thấy ngay trạng thái)
  char msg[160];
  snprintf(msg, sizeof(msg),
           "{\"face\":%d,\"count\":%d,\"http\":%d,\"lat_ms\":%lu}",
           face ? 1 : 0, count, code, dt);

  mqtt.publish(TOPIC_PUB, msg, true);
}

// ====== setup / loop ======
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("WiFi connect...");
  connectBestKnownWiFi(15000);
  ensureWiFiConnected();

  Serial.println("Camera init...");
  cameraInit();

  // TLS gọn nhất (không verify CA). Nếu muốn chuẩn CA cert, mình đưa bản chuẩn.
  tls.setInsecure();
  mqtt.setServer(HIVEMQ_HOST, HIVEMQ_PORT);

  ensureMQTT();
}

void loop() {
  ensureWiFiConnected();
  ensureMQTT();
  mqtt.loop();

  sendFrameToServerAndPublish();
  delay(250);
}
