#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "cJSON.h"
#include "messaging.h"
#include "provisioning.h"
#include "sensor.hpp"

// ---------- sensor.hpp ---------- //
RingbufHandle_t PHBuffer, ECBuffer, RTDBuffer, DOBuffer;
TaskHandle_t PHReadTask, ECReadTask, RTDReadTask, DOReadTask;
SemaphoreHandle_t i2c_mutex, globalTemp_mutex;

QueueHandle_t reportQueue;
TimerHandle_t reportTimer;

// Sensor objects
Ezo_board PH = Ezo_board(99, "PH"); // Where I2C address is 0x99 and sensor name is "PH"
Ezo_board EC = Ezo_board(100, "EC");
Ezo_board RTD = Ezo_board(102, "RTD");

// Array of all sensor info
SensorInfo allSensors[] = {
    {&PHBuffer, &PHReadTask, "pHReadTask", true, &PH, "PH"},
    {&ECBuffer, &ECReadTask, "ECReadTask", true, &EC, "EC"},
    {&RTDBuffer, &RTDReadTask, "RTDReadTask", false, &RTD, "RTD"},
};

const int numSensors = sizeof(allSensors) / sizeof(SensorInfo);

// Global temperature variable
float lastTemp = 25.0;

// ---------- provisioning.h ---------- //

char *nodes[] = {
    "PH",
    "EC",
    "RTD",
};

const int numNodes = sizeof(nodes) / sizeof(nodes[0]);


EventGroupHandle_t adoption_event_group;

TimerHandle_t heartbeatTimer;
int s_retry_num = 0;

char* deviceId;
char* espnowmac;

esp_now_peer_info_t gatewayPeer;
uint8_t* gatewayAddress;
SemaphoreHandle_t mutex;

const char* firmwareVersionClient = "0.0.1";

// ---------- messaging.h ---------- //

esp_now_peer_info_t broadcastPeer;
TaskHandle_t receiveESPNowTaskHandle, sendESPNowTaskHandle;
QueueHandle_t incomingESPNowQueue, outgoingESPNowQueue;

// ---------- led.h ---------- //

rgb_led_t bootLED = {
    LEDC_CHANNEL_0,
    5,
    LEDC_CHANNEL_1,
    6,
    LEDC_CHANNEL_2,
    7};
 
led_t activityLED1 = {LEDC_CHANNEL_6, 8};

// Unused LEDs
rgb_led_t connectivityLED = {};
led_t activityLED2 = {};

// ---------- main.hpp ---------- //
void onESPNowMessagereceiveHandler(cJSON* incomingMessage);
