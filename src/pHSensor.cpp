#include "PhSensor.h"
#include <Wire.h>

PhSensor::PhSensor(const std::string& id, uint8_t i2c_address, uint8_t i2c_bus) 
    : Sensor(id, "pH", i2c_address, i2c_bus) {
    unit = "pH";
    polling_rate = 1000; // for example
    mqtt_topic = "sensor/" + id; // for example
}

void PhSensor::init() {
    // Initialize the sensor, e.g., by sending it an I2C command. 
    // This depends on the specific sensor and its datasheet.

    // For the Atlas Scientific EZO pH sensor, initialization might not be needed.
    // If calibration is needed, it would go here.
}

float PhSensor::readData() {

    // The Atlas Scientific EZO pH sensor has specific I2C commands and responses.
    // You need to follow the protocol described in its datasheet.
    // This is a simplified example:

    Wire.beginTransmission(i2c_address);
    Wire.write("R"); // Send the read command
    Wire.endTransmission();
    
    delay(1000); // wait for sensor to respond
    
    Wire.requestFrom(i2c_address, 20, 1); // request 20 bytes from sensor
    
    byte code = 0;
    String response = "";
    while(Wire.available()) { 
        char c = Wire.read(); // receive a byte
        response += c;
    }
    
    reading = response.toFloat(); // convert the response to a float
    
    return reading;
}
