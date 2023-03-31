// Description: Header file for GPIO class

#ifndef GPIO_H
#define GPIO_H

#include "driver/gpio.h"

// Create class for GPIO pins and their functions
class GPIO {
public:
    // Constructor for GPIO class, takes pin number, mode, pull mode, and interrupt type
    GPIO(gpio_num_t pin, gpio_mode_t mode, gpio_pull_mode_t pull_mode, gpio_int_type_t int_type);

    // Destructor for GPIO class
    ~GPIO();

    // Method to set the level of the GPIO pin. To be used when the pin is set to output, i.e. high or low, no return value
    void set_level(int level);

    // Method to get the level of the GPIO pin. To be used when the pin is set to input, i.e. high or low, returns int
    int get_level();

    // Method to set the direction of the GPIO pin, i.e. input or output, no return value
    void set_direction(gpio_mode_t mode);

    // Method to set the pull mode of the GPIO pin, i.e. pull up or pull down, no return value
    void set_pull_mode(gpio_pull_mode_t pull_mode);

private:
    // Private variable for the pin number
    gpio_num_t pin;

    // Private variable for the mode of the pin
    gpio_mode_t mode;

    // Private variable for the pull mode of the pin
    gpio_pull_mode_t pull_mode;

    // Private variable for the interrupt type of the pin
    gpio_int_type_t int_type;

    // Private variable to cache the level of the pin
    int cached_level;
};

#endif
