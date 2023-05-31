#ifndef PHSENSOR_H
#define PHSENSOR_H

#include "Sensor.h"

class PhSensor : public Sensor {
public:
    PhSensor(const std::string& id, uint8_t i2c_address, uint8_t i2c_bus = 0);
    virtual void init() override;
    virtual float readData() override;
};

#endif
