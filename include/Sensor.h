#ifndef SENSOR_H
#define SENSOR_H

#include <string>

class Sensor {
public:
    Sensor(const std::string& id, const std::string& type, uint8_t i2c_address, uint8_t i2c_bus = 0);
    virtual ~Sensor() {}

    std::string getId() const;
    std::string getType() const;
    uint8_t getI2CAddress() const;
    uint8_t getI2CBus() const;
    float getReading() const;
    std::string getUnit() const;
    uint16_t getPollingRate() const;
    std::string getMqttTopic() const;

    // The init and readData methods will be implemented by the specific sensor subclass.
    virtual void init() = 0;
    virtual float readData() = 0;

protected:
    float reading;
    std::string unit;
    uint16_t polling_rate; // in milliseconds
    std::string mqtt_topic;

private:
    std::string id;
    std::string type;
    uint8_t i2c_address;
    uint8_t i2c_bus;
};

#endif
