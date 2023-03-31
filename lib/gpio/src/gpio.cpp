#include "gpio.h"
#include "esp_log.h"

#define LOG_TAG "GPIO"

GPIO::GPIO(gpio_num_t pin, gpio_mode_t mode, gpio_pull_mode_t pull_mode, gpio_int_type_t int_type) {

    // Constructor for GPIO class, takes pin number, mode, pull mode, and interrupt type
    this->pin = pin;
    this->mode = mode;
    this->pull_mode = pull_mode;
    this->int_type = int_type;
 
    // Configure the pin
    gpio_config_t config;
    config.pin_bit_mask = (1ULL << pin);
    config.mode = mode;
    config.pull_up_en = (pull_mode == GPIO_PULLUP_ONLY || pull_mode == GPIO_PULLUP_PULLDOWN) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    config.pull_down_en = (pull_mode == GPIO_PULLDOWN_ONLY || pull_mode == GPIO_PULLUP_PULLDOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;

    config.intr_type = int_type;
    gpio_config(&config);

    // Set the initial level of the pin
    if (mode == GPIO_MODE_OUTPUT)
    {
        // Set the level of the pin and cache it
        gpio_set_level(pin, 0);
        cached_level = 0;
    }

    // Print a message to the serial console
    ESP_LOGI(LOG_TAG, "Initialized pin %d", pin);

}

GPIO::~GPIO() {
    // Nothing to do here
}

void GPIO::set_level(int level) {
    // Check if pin is set to output mode, print error if not
    if (mode != GPIO_MODE_OUTPUT)
    {
        // Print error message
        ESP_LOGW(LOG_TAG, "Cannot set level of pin %d, it is set to input mode", pin);
        return;
    }

    // Set the level of the pin and cache it
    gpio_set_level(pin, level);
    cached_level = level;
}

int GPIO::get_level() {
    // Check if pin is set to input mode, print error and return 0 if not
    if (mode != GPIO_MODE_INPUT)
    {
        // Print error message
        ESP_LOGW(LOG_TAG, "Cannot get level of pin %d, it is set to output mode", pin);
        return 0;
    }
    // else, return pin level
    return gpio_get_level(pin);
}

