// Firmware for ESP32 Based Hydroponics Controller

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

// Hardcoded Wifi and MQTT Settings
#define WIFI_SSID "SigmaNet"
#define WIFI_PASSWORD "YourPassword"

#define MQTT_SERVER "mqtt://your-mqtt-server"
#define MQTT_PORT 1883


void my_setup()
{
  // Setup Wifi

  // Create Sensor Objects

  // Create new RTOS tasks for each sensor

  // Setup MQTT


}



extern "C" void app_main(void)
{
  my_setup();

  while(1)
  {
    sensor_loop();
  }
  
}

