//************************************************************
//
//************************************************************

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
//#include "esp_spiffs.h"
#include "esp_event.h"
//#include "esp_log.h"
//#include "nvs.h"
//#include "nvs_flash.h"


#include "gateway_config.h"
#include <fdrs_gateway.h>


void my_setup()
{
beginFDRS();
initArduino();
//Serial.begin(115200);

}

extern "C" void app_main(void)
{
  my_setup();

  while(1)
  {
    loopFDRS();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  
}





