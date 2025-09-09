#include <Arduino.h>
#include "FileReader.h"
#include <I2CSoilMoistureSensor.h>
#include <I2CScanner.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

const char* ssid     = "Your_SSID";
const char* password = "Your_PASSWORD";

FileReader fileReader;
WiFiClientSecure client;

uint8_t sensorAddress = 0x20;
I2CSoilMoistureSensor sensor(sensorAddress);

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

  Serial.printf("Connecting to %s...", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setCACert(caCert.c_str());
  client.setCertificate(sensorCert.c_str());
  client.setPrivateKey(sensorKey.c_str());
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