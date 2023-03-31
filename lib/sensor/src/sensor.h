// defining a header file in c++ for sensor class in ESP IDF framework

#ifndef SENSOR_H
#define SENSOR_H

// Creating enum class for sensor type
enum class SensorType {
    // DHT11 sensor and nothing else for now
    DHT11
};

// Creating enum class for sensor mode
enum class SensorMode {
    //
    SINGLE_SHOT,
    CONTINUOUS
};

// Sensor class
class Sensor {
    public:
        Sensor {
            // constructor
            int address,
            // Using const char *name instead of std::string name 
            // Because its significantly more efficient in cpp compile size
            const char *name,
            SensorType type, SensorMode mode
        }
    private:   
}

// end of header file