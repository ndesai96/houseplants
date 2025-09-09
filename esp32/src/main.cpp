#include <Arduino.h>
#include "FileReader.h"
#include <I2CSoilMoistureSensor.h>
#include <I2CScanner.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

const char* ssid     = "Your_SSID";
const char* password = "Your_PASSWORD";

const char* mqttServer = "192.168.1.133"; // use ipconfig getifaddr en0 to find your local IP address
const char* mqttTopic = "houseplants/esp32";

uint8_t sensorAddress = 0x20;
I2CSoilMoistureSensor sensor(sensorAddress);

FileReader fileReader;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void setCerts() {
  if (!fileReader.begin()) {
    Serial.println("Failed to initialize file system");
    while (1);
  }

  String caCert = fileReader.readFile("/ca.crt");
  if (caCert.isEmpty()) {
    Serial.println("Failed to read ca.crt");
    while (1);
  }

  String sensorCert = fileReader.readFile("/sensor.crt");
  if (sensorCert.isEmpty()) {
    Serial.println("Failed to read sensor.crt");
    while (1);
  }

  String sensorKey = fileReader.readFile("/sensor.key");
  if (sensorKey.isEmpty()) {
    Serial.println("Failed to read sensor.key");
    while (1);
  }

  wifiClient.setCACert(caCert.c_str());
  wifiClient.setCertificate(sensorCert.c_str());
  wifiClient.setPrivateKey(sensorKey.c_str());
}

void connectToWiFi() {
  Serial.printf("Connecting to %s...", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("success");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  mqttClient.setServer(mqttServer, 8883);
  
  char clientId[32];
  snprintf(clientId, sizeof(clientId), "ESP32-%08X", (unsigned int)esp_random());

  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect(clientId)) {
      Serial.println("success");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);
  sensor.begin();

  if (!sensor.validateAddress()) {
    Serial.print("Sensor not found at address 0x");
    Serial.println(sensorAddress, HEX);
    while (1);
  } else {
    Serial.print("Current Sensor I2C address: 0x");
    Serial.println(sensor.getAddress(), HEX);
  }

  Serial.print("Sensor Firmware version: ");
  Serial.println(sensor.getVersion(), HEX);

  setCerts();
  connectToWiFi();
}

void loop() {
  unsigned int moisture = sensor.getMoisture();
  float temperature = sensor.getTemperature() / 10.0;
  unsigned int light = sensor.getLight();

  Serial.print("Moisture: ");
  Serial.print(moisture);

  Serial.print(", Temperature: ");
  Serial.print(temperature);
  Serial.print("°C");

  Serial.print(", Light: ");
  Serial.print(light);

  Serial.println();

  delay(1000); // Wait 1 second before the next scan
}