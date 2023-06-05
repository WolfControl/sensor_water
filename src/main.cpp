// Firmware for ESP32 Based Hydroponics Controller

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/semphr.h"


#include "WiFi.h"
#include <PubSubClient.h>

#include <Ezo_i2c.h>
#include <Wire.h>
#include "nvs_flash.h"

// Global constants, vars, and initializations
#define WIFI_SSID "MyNetwork"
#define WIFI_PASSWORD "MyPassword"

#define MQTT_SERVER "mqtt://your-mqtt-server"
#define MQTT_PORT 1883

SemaphoreHandle_t i2c_semaphore = NULL;

Ezo_board PH = Ezo_board(99, "PH");
//Ezo_board EC = Ezo_board(100, "EC");


struct Device {
  Ezo_board board; // How to be agnostic of ezo_board library?
  int pollingRate; // in ms
  const char* mqttTopic; // topic to publish to
};

Device device_list[] = {
  {Ezo_board(99, "PH"), 1000, "hydroponics/ph"},
  //{Ezo_board(100, "EC"), 2000, "hydroponics/ec"},
  // add more devices here...
};

void wifi_setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  ESP_LOGI("WIFI", "Attempting to connect to %s", WIFI_SSID);
  while(WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500);
    ESP_LOGI("WIFI", "Connecting to %s", WIFI_SSID);
  }

  ESP_LOGI("WIFI", "Connected to %s", WIFI_SSID);
  ESP_LOGI("WIFI", "IP Address: %s", WiFi.localIP().toString().c_str());

  //TODO: failure and retry handling

}

void mqtt_setup()
{
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);

  //TODO: failure and retry handling
}

void sensor_loop(void* parameter)
{
  Device* device = (Device*) parameter;
  char result[30];

  while(1) {
    xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
    device->board.send_read_cmd();
    xSemaphoreGive(i2c_semaphore);

    device->board.receive_read_cmd();
    float reading = device->board.get_last_received_reading();
    sprintf(result, "%.2f", reading); // convert float to string

    mqtt_client.publish(device->mqttTopic, result);
    ESP_LOGI("SENSOR", "Published reading: %s = %s", device->mqttTopic, result);

    vTaskDelay(device->pollingRate / portTICK_PERIOD_MS);
  }
}




void my_setup()
{
  wifi_setup();

  mqtt_setup();

  // Create new RTOS tasks for each sensor
  for (int i = 0; i < sizeof(device_list); i++)
  {
    //xTaskCreate
  }
}

extern "C" void app_main(void)
{
  my_setup();

  while(1)
  {

  }
  
}
