#include "I2CSoilMoistureSensor.h"
#include <Wire.h>

I2CSoilMoistureSensor::I2CSoilMoistureSensor(uint8_t address) : _address(address) {}

void I2CSoilMoistureSensor::begin() {
    Wire.begin();
    delay(100); // Allow some time for the sensor to initialize
    reset();
}

bool I2CSoilMoistureSensor::validateAddress() {
    return (readRegister8(GET_ADDRESS) == _address);
}

bool I2CSoilMoistureSensor::setAddress(int newAddress) {
    if (newAddress < 1 || newAddress > 127) {
        return false; // Invalid I2C address
    }
    writeRegister8(SET_ADDRESS, newAddress);
    writeRegister8(SET_ADDRESS, newAddress);

    reset();
    _address = newAddress;
    return (readRegister8(GET_ADDRESS) == newAddress);
}

uint8_t I2CSoilMoistureSensor::getAddress() {
    return _address;
}

void I2CSoilMoistureSensor::reset() {
    writeRegister8(RESET);
    delay(1000); // Allow some time for the sensor to reset
}

uint8_t I2CSoilMoistureSensor::getVersion() {
    return readRegister8(GET_VERSION);
}

void I2CSoilMoistureSensor::writeRegister8(int reg) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.endTransmission();
}

void I2CSoilMoistureSensor::writeRegister8(int reg, int value) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t I2CSoilMoistureSensor::readRegister8(int reg) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.endTransmission();
    delay(20);
    Wire.requestFrom(_address, uint8_t(1));
    return Wire.read();
}

uint16_t I2CSoilMoistureSensor::readRegister16(int reg) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.endTransmission();
    delay(20);
    Wire.requestFrom(_address, uint8_t(2));
    return (int16_t)((Wire.read() << 8) | Wire.read());
}