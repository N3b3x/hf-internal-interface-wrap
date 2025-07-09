#pragma once
#include "EspI2c.h"

class DummyDevice {
public:
  DummyDevice(EspI2c &bus, uint8_t address) : bus(bus), address(address) {}
  bool Init() {
    uint8_t id = 0;
    return bus.WriteRead(address, nullptr, 0, &id, 1);
  }

private:
  EspI2c &bus;
  uint8_t address;
};
