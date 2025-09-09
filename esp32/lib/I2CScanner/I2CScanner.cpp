#include "I2CScanner.h"
#include <Arduino.h>
#include <Wire.h>

void scan() {
    byte error, address;
    int nDevices;

    Wire.begin();
    Serial.println("Scanning for I2C devices...");

    nDevices = 0;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            nDevices++;
        } else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }

    if (nDevices == 0) {
        Serial.println("No I2C devices found\n");
    } else {
        Serial.print(nDevices);
        Serial.println(" I2C devices found.");
    }

    Serial.println();
}