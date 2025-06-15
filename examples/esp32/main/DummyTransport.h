#ifndef DUMMY_TRANSPORT_H
#define DUMMY_TRANSPORT_H

#include "I2cBus.h"

class DummyTransport {
  public:
    explicit DummyTransport(I2cBus &b) : bus(b) {}
    bool WriteReg(uint8_t reg, uint8_t val);
    bool ReadReg(uint8_t reg, uint8_t &val);

  private:
    I2cBus &bus;
};

#endif // DUMMY_TRANSPORT_H
