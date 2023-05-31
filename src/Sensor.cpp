#include "Sensor.h"

Sensor::Sensor(const std::string& id, const std::string& type, uint8_t i2c_address, uint8_t i2c_bus)
    : id(id), type(type), i2c_address(i2c_address), i2c_bus(i2c_bus) {
}

std::string Sensor::getId() const {
    return id;
}

std::string Sensor::getType() const {
    return type;
}

uint8_t Sensor::getI2CAddress() const {
    return i2c_address;
}

uint8_t Sensor::getI2CBus() const {
    return i2c_bus;
}

float Sensor::getReading() const {
    return reading;
}

std::string Sensor::getUnit() const {
    return unit;
}

uint16_t Sensor::getPollingRate() const {
    return polling_rate;
}

std::string Sensor::getMqttTopic() const {
    return mqtt_topic;
}
