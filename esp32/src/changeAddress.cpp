#include <Arduino.h>
#include <I2CSoilMoistureSensor.h>
#include <I2CScanner.h>
#include <Wire.h>

uint8_t currentAddress = 0x21; // Current I2C address
uint8_t newAddress = 0x20; // New I2C address to set
I2CSoilMoistureSensor sensor(currentAddress);

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(100);
    sensor.begin();
    delay(1000);
    
    if (!sensor.validateAddress()) {
        Serial.print("Sensor not found at address 0x");
        Serial.println(currentAddress, HEX);
        while (1);
    } else {
        Serial.print("Current Sensor I2C address: 0x");
        Serial.println(sensor.getAddress(), HEX);
    }

    if (sensor.setAddress(newAddress)) {
        Serial.print("I2C address changed successfully to 0x");
        Serial.println(newAddress, HEX);
    } else {
        Serial.println("Failed to change I2C address.");
        while (1);
    }

    Serial.println();
}

void loop() {
    scan();
    delay(5000); // Wait 5 seconds before the next scan
}