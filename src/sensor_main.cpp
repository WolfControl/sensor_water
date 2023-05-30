//  FARM DATA RELAY SYSTEM
//
//  ESP-NOW Sensor Example
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//  An example of how to send data via ESP-NOW using FDRS.
//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
//#include "esp_spiffs.h"
#include "esp_event.h"
#include "sensor_config.h"
#include <fdrs_node.h>

float data1;
float data2;

float readTemp() {
  return 22.069;
}

float readHum() {
  return random(0, 100);
}

void my_setup()
{
beginFDRS();
initArduino();
//Serial.begin(115200);

}

void sensor_loop() {
  data1 = readHum();
  loadFDRS(data1, HUMIDITY_T);
  data2 = readTemp();
  loadFDRS(data2, TEMP_T);
//  DBG(sendFDRS());
    if(sendFDRS()){
      DBG("Big Success!");
    } else {
      DBG("Nope, not so much.");
    }
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  //sleepFDRS(10);  //Sleep time in seconds
}



extern "C" void app_main(void)
{
  my_setup();

  while(1)
  {
    sensor_loop();
  }
  
}

