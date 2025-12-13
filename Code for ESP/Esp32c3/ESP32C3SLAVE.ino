#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define OLED_SDA 8
#define OLED_SCL 9
#define OLED_ADDR 0x3C

WiFiMulti wifiMulti;

const char* mqtt_server   = "3a73d99203e7455aa6e3f7e90aea1e6e.s1.eu.hivemq.cloud";
const int   mqtt_port     = 8883;
const char* mqtt_username = "Slave_ESP32C3";
const char* mqtt_password = "@Hana4869";

WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long timeUpdate = 0;

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

  randomSeed(micros());
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    if(wifiMulti.run() != WL_CONNECTED) {
      delay(500);
      continue;
    }

    Serial.print("Attempting MQTT connection...");
    String clientID = "ESP32C3-";
    clientID += String(random(0xffff), HEX);

    if (client.connect(clientID.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("esp32c3/client");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" -> try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String incomingMessage = "";
  for (unsigned int i = 0; i < length; i++) {
    incomingMessage += (char)payload[i];
  }
  Serial.println("Message arrived [" + String(topic) + "]: " + incomingMessage);
}

void publishMessage(const char* topic, String payload, boolean retained) {
  if (client.publish(topic, payload.c_str(), retained)) {
    Serial.println("Message published [" + String(topic) + "]: " + payload);
  } else {
    Serial.println("Publish failed!");
  }
}

void updateOLED(float t, float h) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ESP32-C3 DHT11");

  display.setCursor(0, 16);
  display.print("Temp: ");
  display.print(t, 1);
  display.println(" C");

  display.setCursor(0, 28);
  display.print("Hum : ");
  display.print(h, 1);
  display.println(" %");

  display.setCursor(0, 40);
  display.print("WiFi: ");
  if (WiFi.status() == WL_CONNECTED) {
    display.println("OK");
  } else {
    display.println("DIS");
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Booting...");
  display.display();

  dht.begin();
  setup_wifi();

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  timeUpdate = millis();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi connected");
  display.setCursor(0, 10);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(1500);
}

void loop() {
  if(wifiMulti.run() != WL_CONNECTED) {
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - timeUpdate > 5000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      updateOLED(0, 0);
      timeUpdate = millis();
      return;
    }

    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" C  -  Hum: ");
    Serial.print(h);
    Serial.println(" %");

    updateOLED(t, h);

    DynamicJsonDocument doc(1024);
    doc["humidity"]    = h;
    doc["temperature"] = t;

    char mqtt_message[128];
    serializeJson(doc, mqtt_message);

    publishMessage("esp32c3/dht11", mqtt_message, true);

    timeUpdate = millis();
  }
}