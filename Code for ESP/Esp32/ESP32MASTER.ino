#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN   19
#define DHTTYPE  DHT22
DHT dht(DHTPIN, DHTTYPE);

#define PIN_RL1  14
#define PIN_RL2  12
#define PIN_RL3  13

WiFiMulti wifiMulti;

const char* mqtt_server   = "3a73d99203e7455aa6e3f7e90aea1e6e.s1.eu.hivemq.cloud";
const int   mqtt_port     = 8883;
const char* mqtt_username = "Master_ESP32";
const char* mqtt_password = "@Hana4869";

WiFiClientSecure espClient;
PubSubClient client(espClient);

float remoteTemp = NAN;
float remoteHum  = NAN;

unsigned long lastReadLocal = 0;
const unsigned long READ_INTERVAL = 5000;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.println("Setting up WiFi...");

  wifiMulti.addAP("MILANO *KIMSON", "77777777#");
  wifiMulti.addAP("Tường Vinh", "hana4869");
  wifiMulti.addAP("Xiaomi 14T pro", "23111124");
  wifiMulti.addAP("Bao Nghia", "27132713");

  Serial.println("Connecting to WiFi...");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void publishEnv(float localTemp, float localHum, float rTemp, float rHum) {
  StaticJsonDocument<256> doc;

  JsonObject local = doc.createNestedObject("local");
  if (isnan(localTemp)) {
    local["temp"] = nullptr;
  } else {
    local["temp"] = localTemp;
  }
  if (isnan(localHum)) {
    local["hum"] = nullptr;
  } else {
    local["hum"] = localHum;
  }

  JsonObject remote = doc.createNestedObject("remote");
  if (isnan(rTemp)) {
    remote["temp"] = nullptr;
  } else {
    remote["temp"] = rTemp;
  }
  if (isnan(rHum)) {
    remote["hum"] = nullptr;
  } else {
    remote["hum"] = rHum;
  }

  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  if (client.publish("esp32server/env", buffer, true)) {
    Serial.print("Published ENV: ");
    Serial.println(buffer);
  } else {
    Serial.println("Publish ENV failed");
  }
}

void publishRelayState() {
  StaticJsonDocument<128> doc;

  doc["rl1"] = digitalRead(PIN_RL1) ? 1 : 0;
  doc["rl2"] = digitalRead(PIN_RL2) ? 1 : 0;
  doc["rl3"] = digitalRead(PIN_RL3) ? 1 : 0;

  char buffer[128];
  size_t n = serializeJson(doc, buffer);

  if (client.publish("esp32server/relay", buffer, true)) {
    Serial.print("Published RELAY: ");
    Serial.println(buffer);
  } else {
    Serial.println("Publish RELAY failed");
  }
}

void handleRelayCommand(byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("JSON relay cmd parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  Serial.println("Handling relay command...");

  if (doc.containsKey("rl1")) {
    int v = doc["rl1"];
    digitalWrite(PIN_RL1, v ? HIGH : LOW);
    Serial.print("  RL1 -> ");
    Serial.println(v ? "ON" : "OFF");
  }

  if (doc.containsKey("rl2")) {
    int v = doc["rl2"];
    digitalWrite(PIN_RL2, v ? HIGH : LOW);
    Serial.print("  RL2 -> ");
    Serial.println(v ? "ON" : "OFF");
  }

  if (doc.containsKey("rl3")) {
    int v = doc["rl3"];
    digitalWrite(PIN_RL3, v ? HIGH : LOW);
    Serial.print("  RL3 -> ");
    Serial.println(v ? "ON" : "OFF");
  }

  publishRelayState();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);

  Serial.print("Message arrived [");
  Serial.print(topicStr);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (topicStr == "esp32c3/dht11") {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
      return;
    }

    remoteHum  = doc["humidity"]    | NAN;
    remoteTemp = doc["temperature"] | NAN;

    Serial.print("-> Remote DHT11 updated: ");
    Serial.print(remoteTemp);
    Serial.print(" C, ");
    Serial.print(remoteHum);
    Serial.println(" %");
  }
  else if (topicStr == "esp32server/relay/cmd") {
    handleRelayCommand(payload, length);
  }
}

void reconnect() {
  while (!client.connected()) {
    if(wifiMulti.run() != WL_CONNECTED) {
      delay(500);
      continue;
    }

    Serial.print("Attempting MQTT connection...");
    String clientID = "ESP32-SERVER-";
    clientID += String(random(0xffff), HEX);

    if (client.connect(clientID.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("esp32c3/dht11");
      client.subscribe("esp32server/relay/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" -> try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }

  pinMode(PIN_RL1, OUTPUT);
  pinMode(PIN_RL2, OUTPUT);
  pinMode(PIN_RL3, OUTPUT);
  digitalWrite(PIN_RL1, LOW);
  digitalWrite(PIN_RL2, LOW);
  digitalWrite(PIN_RL3, LOW);

  dht.begin();

  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  lastReadLocal = millis();

  Serial.println("ESP32 MASTER ready");
}

void loop() {
  if(wifiMulti.run() != WL_CONNECTED) {
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - lastReadLocal > READ_INTERVAL) {
    float localHum  = dht.readHumidity();
    float localTemp = dht.readTemperature();

    Serial.println("====================================");

    if (isnan(localHum) || isnan(localTemp)) {
      Serial.println("[LOCAL DHT22] Failed to read sensor!");
      localHum  = NAN;
      localTemp = NAN;
    } else {
      Serial.print("[LOCAL DHT22] Temp: ");
      Serial.print(localTemp);
      Serial.print(" C  -  Hum: ");
      Serial.print(localHum);
      Serial.println(" %");
    }

    if (isnan(remoteTemp) || isnan(remoteHum)) {
      Serial.println("[REMOTE DHT11] No data from ESP32-C3.");
    } else {
      Serial.print("[REMOTE DHT11] Temp: ");
      Serial.print(remoteTemp);
      Serial.print(" C  -  Hum: ");
      Serial.print(remoteHum);
      Serial.println(" %");
    }

    Serial.println("====================================");

    publishEnv(localTemp, localHum, remoteTemp, remoteHum);
    publishRelayState();

    lastReadLocal = millis();
  }
}