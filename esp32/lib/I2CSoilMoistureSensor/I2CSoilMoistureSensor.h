#ifndef I2CSOILMOISTURESENSOR_H
#define I2CSOILMOISTURESENSOR_H

// Default I2C Address of the sensor
#define DEFAULT_ADDR 0x20

// Register Addresses of the sensor
#define GET_CAPACITANCE 0x00 // (r) 2 bytes
#define SET_ADDRESS 	0x01 // (w) 1 byte
#define GET_ADDRESS 	0x02 // (r) 1 byte
#define MEASURE_LIGHT 	0x03 // (w) n/a
#define GET_LIGHT 		0x04 // (r) 2 bytes
#define GET_TEMPERATURE 0x05 // (r) 2 bytes
#define RESET 			0x06 // (w) n/a
#define GET_VERSION 	0x07 // (r) 1 bytes
#define SLEEP			0x08 // (w) n/a
#define GET_BUSY		0x09 // (r) 1 bytes

#include <Arduino.h>

class I2CSoilMoistureSensor {
    public:
        I2CSoilMoistureSensor(uint8_t address = DEFAULT_ADDR);
        void begin();
        bool validateAddress();

        unsigned int getMoisture();
        bool setAddress(int newAddress);
        uint8_t getAddress();
        void measureLight();
        unsigned int getLight();
        int getTemperature();
        void reset();
        uint8_t getVersion();
        void sleep();
        bool isBusy();

    private:
        uint8_t _address;

        void writeRegister8(int reg);
        void writeRegister8(int reg, int value);
        uint8_t readRegister8(int reg);
        uint16_t readRegister16(int reg);
};

#endif // I2CSOILMOISTURESENSOR_H

