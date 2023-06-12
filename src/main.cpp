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
#include "cJSON.h"

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


// Create a struct_message to send to the ESP NOW receiver
typedef struct struct_message {
  char topic[50];      // MQTT topic to publish to
  float sensorValue;   // Sensor reading
  char sensorName[30]; // Name of the sensor
  char deviceName[30]; // Name or ID of the device sending the message
  uint32_t timestamp;  // Unix timestamp (seconds since 1970-01-01 00:00:00 UTC)
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a variable to hold a pointer to the peer
esp_now_peer_info_t peerInfo;


/*---------- Callback Functions ----------*/

// Callback function that will be executed when data is sent over ESP NOW
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  static const char *TAG = "OnDataSent";

  ESP_LOGI(TAG, "Sent message struct ESP-NOW receiver");
  ESP_LOGI(TAG, "Last Packet Send Status: %d", status == ESP_NOW_SEND_SUCCESS);

}

/*---------- Setup Functions ----------*/

void os_setup()
{
    ESP_LOGI("OS", "Creating default event loop...");
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void flash_setup()
{

    ESP_LOGI("NVS", "Initializing NVS flash...");
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_LOGW("NVS", "No free pages OR new version found. Erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());

        ESP_LOGI("NVS", "Initializing NVS flash again...");
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("NVS", "Flash initialized.");
}

void network_setup()
{
  
      ESP_LOGI("WIFI", "Initializing tcpip adapter...");
      ESP_ERROR_CHECK(esp_netif_init());
  
      ESP_LOGI("WIFI", "Initializing default station...");
      esp_netif_create_default_wifi_sta();
  
      wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
      ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  
      ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
      ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

      ESP_ERROR_CHECK(esp_wifi_start());

      if (esp_now_init() != ESP_OK) {
        ESP_LOGE("ESP-NOW", "Error initializing ESP-NOW");
        return;
  }
}

void my_setup()
{

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
  
}

/*---------- Feature Functions ----------*/

/*
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

*/


void send_fakedata(void* parameter)
{
  static const char *TAG = "send_fakedata";
    while(1) {
        // Create a cJSON object
        cJSON* json = cJSON_CreateObject();

        // Add data to the cJSON object
        cJSON_AddStringToObject(json, "topic", "ESPGateway/from/FakeSensor");
        cJSON_AddNumberToObject(json, "sensorValue", (rand() % 100) / 10.0);
        cJSON_AddStringToObject(json, "sensorName", "PH");
        cJSON_AddStringToObject(json, "deviceName", "Feather_V2");
        cJSON_AddNumberToObject(json, "timestamp", (uint32_t)time(NULL)); // Get the current time as a Unix timestamp

        // Convert the cJSON object to a JSON string
        char* jsonString = cJSON_Print(json);

        // Send the JSON string over ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) jsonString, strlen(jsonString) + 1); // +1 to include null terminator

        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Published reading to ESP-NOW: %s", jsonString);
        }
        else {
            ESP_LOGW(TAG, "Error: %s", esp_err_to_name(result));
        }

        // Free the JSON string and cJSON object
        free(jsonString);
        cJSON_Delete(json);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}


/*---------- Main Function ----------*/

extern "C" void app_main(void)
{
  static const char *TAG = "app_main";

  ESP_LOGI(TAG, "Call OS setup");
  os_setup();

  ESP_LOGI(TAG, "Call flash setup");
  flash_setup();

  ESP_LOGI(TAG, "Call network setup");
  network_setup();

  ESP_LOGI(TAG, "Call my setup");
  my_setup();

  ESP_LOGI("DEBUG", "Spawn task to generate and send fake data to ESP-NOW gateway");
  xTaskCreate(send_fakedata, "send_fakedata", 2048, NULL, 5, NULL);

  // Create new RTOS tasks for each sensor
  //for (int i = 0; i < sizeof(device_list); i++)
  //{
    //ESP_LOGI(TAG, "Creating task for %s", device_list[i].board.get_name());
    //xTaskCreate(sensor_loop, device_list[i].board.get_name(), 2048, &device_list[i], 5, NULL);
  //}
  
  
}
