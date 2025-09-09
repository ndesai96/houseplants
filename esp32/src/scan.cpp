#include <Arduino.h>
#include <I2CScanner.h>

void setup() {
    Serial.begin(115200);
}

void loop() {
    scan();
    delay(5000); // Wait 5 seconds before the next scan
}