#include "DummyTransport.h"

bool DummyTransport::WriteReg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    return bus.Write(0x50, buf, 2);
}

bool DummyTransport::ReadReg(uint8_t reg, uint8_t &val) {
    return bus.WriteRead(0x50, &reg, 1, &val, 1);
}
