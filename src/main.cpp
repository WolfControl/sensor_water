#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include <Ezo_i2c.h>
#include <Wire.h>
#include "nvs_flash.h"

/*---------- Sensor Related Globals & Stubs ----------*/
SemaphoreHandle_t i2c_semaphore = NULL;

Ezo_board PH = Ezo_board(99, "PH");
//Ezo_board EC = Ezo_board(100, "EC");

struct Device {
  Ezo_board board; // How to be agnostic of ezo_board library?
  int pollingRate; // in ms
  const char* mqttTopic; // topic to publish to
};

Device device_list[] = {
  {Ezo_board(99, "PH"), 1000, "hydroponics/ph"}
  //{Ezo_board(100, "EC"), 2000, "hydroponics/ec"},
};

/*---------- ESP NOW Related Globals & Stubs ----------*/

// Address of the ESP NOW receiver
uint8_t broadcastAddress[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};

// Structure of the data to be sent
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a variable to hold a pointer to the peer
esp_now_peer_info_t peerInfo;


/*---------- Callback Functions ----------*/

// Callback function that will be executed when data is sent over ESP NOW
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/*---------- Other Functions ----------*/

void sensor_loop(void* parameter)
{
  Device* device = (Device*) parameter;
  char result[30];

  while(1) {
    xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
    device->board.send_read_cmd();

    device->board.receive_read_cmd();
    float reading = device->board.get_last_received_reading();

    // Release the I2C bus
    xSemaphoreGive(i2c_semaphore);

    sprintf(result, "%.2f", reading); // convert float to string

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      ESP_LOGI("SENSOR", "Published reading: FILL IN HERE");
    }
    else {
      ESP_LOGW("SENSOR", "Error sending the data");
    }
    
    }
    vTaskDelay(device->pollingRate / portTICK_PERIOD_MS);
}

void my_setup()
{

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

  if (esp_now_init() != ESP_OK) {
    ESP_LOGE("ESP-NOW", "Error initializing ESP-NOW");
    return;
  }

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  //Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  //Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGE("ESPNOW", "Failed to add peer");
    return;
  }


  // Create new RTOS tasks for each sensor
  for (int i = 0; i < sizeof(device_list); i++)
  {
    xTaskCreate(sensor_loop, device_list[i].board.get_name(), 2048, &device_list[i], 5, NULL);
  }
  
  
}

extern "C" void app_main(void)
{
  my_setup();

  //while(1)
  //{

  //}
  
}
