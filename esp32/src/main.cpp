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

const char* mqttServer = "192.168.1.137"; // use ipconfig getifaddr en0 to find your local IP address
const char* mqttTopic = "houseplants/esp32";

// Interval to wait for light measurement to complete
const long waitForLightInterval = 3000; // 3 seconds

// Interval to wait between sensor reads
// Must be greater than waitForLightInterval
const long readInterval = 10000; // 10 seconds

char clientId[32];

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

  const char* caCert = fileReader.readFile("/ca.crt");
  if (caCert == nullptr) {
    Serial.println("Failed to read ca.crt");
    while (1);
  }

  const char* sensorCert = fileReader.readFile("/sensor.crt");
  if (sensorCert == nullptr) {
    Serial.println("Failed to read sensor.crt");
    while (1);
  }

  const char* sensorKey = fileReader.readFile("/sensor.key");
  if (sensorKey == nullptr) {
    Serial.println("Failed to read sensor.key");
    while (1);
  }

  wifiClient.setCACert(caCert);
  wifiClient.setCertificate(sensorCert);
  wifiClient.setPrivateKey(sensorKey);
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

  while (!mqttClient.connected()) {
    Serial.printf("Connecting to MQTT broker %s...", mqttServer);
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

  snprintf(clientId, sizeof(clientId), "ESP32-%08X", (unsigned int)esp_random());

  setCerts();
  connectToWiFi();
}

bool publishData(unsigned int moisture, float temperature, unsigned int light) {
  String payload = "{";
  payload += "\"moisture\":" + String(moisture) + ",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"light\":" + String(light);
  payload += "}";
  return mqttClient.publish(mqttTopic, payload.c_str());
}

enum State {
  WAITING_FOR_INTERVAL,
  WAITING_FOR_LIGHT,
  READING_SENSORS,
  PUBLISHING_DATA,
};

State currentState = WAITING_FOR_INTERVAL;

unsigned int moisture;
float temperature;
unsigned int light;

unsigned long lastSensorRead = 0;
unsigned long startedMeasuringLight = 0;

void loop() {
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();

  unsigned long currentTime = millis();

  switch (currentState) {
    case WAITING_FOR_INTERVAL: {
      if (currentTime - lastSensorRead >= readInterval - waitForLightInterval) {
        startedMeasuringLight = currentTime;
        sensor.measureLight();
        currentState = WAITING_FOR_LIGHT;
      }
      break;
    }

    case WAITING_FOR_LIGHT: {
      if (currentTime - startedMeasuringLight >= waitForLightInterval) {
        currentState = READING_SENSORS;
      }
      break;
    }

    case READING_SENSORS: {
      moisture = sensor.getMoisture();
      temperature = sensor.getTemperature() / 10.0;
      light = sensor.getLight(false);
      currentState = PUBLISHING_DATA;
      lastSensorRead = currentTime;
      break;
    }

    case PUBLISHING_DATA: {
      if (!publishData(moisture, temperature, light)) {
        Serial.print("Failed to publish data to ");
        Serial.println(mqttTopic);
      }
      currentState = WAITING_FOR_INTERVAL;
      break;
    }
  }
}