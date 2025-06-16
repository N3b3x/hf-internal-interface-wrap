#pragma once
#include "I2cBus.h"

class DummyDevice {
public:
    DummyDevice(I2cBus& bus, uint8_t address) : bus(bus), address(address) {}
    bool Init() {
        uint8_t id = 0;
        return bus.WriteRead(address, nullptr, 0, &id, 1);
    }
private:
    I2cBus& bus;
    uint8_t address;
};
