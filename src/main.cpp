#include <stdio.h>
#include <string.h>

// Required for using GPIO pins
#include "driver/gpio.h"

// Realtime OS - multithreading, wait tasks, etc
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// ESP hardware specific libs
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

// Storing key value pairs in flash memory
#include "nvs_flash.h"

// Wireless networking
#include "esp_wifi.h"
#include "esp_smartconfig.h"

// Lightweight TCP/IP stack
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

// Include my custom GPIO class
#include "gpio.h"


#define LOG_TAG "MAIN"

// Entrypoint to FreeRTOS, i.e. main thread
extern "C" void app_main(void) 
{

    ESP_LOGI(LOG_TAG, "Creating default event loop");
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(LOG_TAG, "Initializing flash memory");
    ESP_ERROR_CHECK(nvs_flash_init());

    // Create GPIO object for pin 22
    GPIO gpio22(GPIO_NUM_22, GPIO_MODE_INPUT, GPIO_PULLDOWN_ONLY, GPIO_INTR_DISABLE);

    // Create GPIO object for pin 2
    GPIO gpio2(GPIO_NUM_2, GPIO_MODE_OUTPUT, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);

    while(true)
    {

        // move this to its own loop and call it here
        if (gpio22.get_level() == 0)
        {
            // Sets pin 2 to low and prints to serial console
            gpio2.set_level(0);
        } 
        else
        {
            // Sets pin 2 to high
            gpio2.set_level(1);
            ESP_LOGI(LOG_TAG, "BingBong");

        }

        // Add delay at the end of loop so as not to overwhelm serial console
        vTaskDelay(100);
    }
}
