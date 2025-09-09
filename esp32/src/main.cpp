#include <Arduino.h>
#include <I2CSoilMoistureSensor.h>
#include <I2CScanner.h>
#include <Wire.h>

uint8_t sensorAddress = 0x20;
I2CSoilMoistureSensor sensor(sensorAddress);

void setup() {
  Serial.begin(115200);
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
  Serial.println();
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