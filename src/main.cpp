//************************************************************
// this is a MqttBroker example that uses the painlessMesh library
//
// connect to a another network and relay messages from a MQTT broker to the nodes of the mesh network.
//
// - To send a message to a mesh node, you can publish it to "painlessMesh/to/NNNN" where NNNN equals the nodeId.
// - To broadcast a message to all nodes in the mesh you can publish it to "painlessMesh/to/broadcast".
// - When you publish "getNodes" to "painlessMesh/to/gateway" you receive the mesh topology as JSON
//
// - Every message from the mesh which is sent to the gateway node will be published to "painlessMesh/from/NNNN" where NNNN
//   is the nodeId from which the packet was sent.
//
//
//************************************************************

#include <Arduino.h>
#include "painlessMesh.h"
#include "PubSubClient.h"
#include "WifiClient.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"


// PainlessMesh credentials ( name, password and port )
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// WiFi credentials: should match your access point!
#define   STATION_SSID     "SigmaNet"
#define   STATION_PASSWORD "Octagon1"

#define   HOSTNAME         "MQTT_Bridge"

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);


IPAddress getlocalIP();
IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 1, 13);

#define mqttPort 1883


// topic's suffix: everyone can publish/subscribe to this public broker,
// you have to change the following 2 defines
#define PUBPLISHSUFFIX             "CollectorMesh/from/"
#define SUBSCRIBESUFFIX            "CollectorMesh/to/"

#define PUBPLISHFROMGATEWAYSUFFIX  PUBPLISHSUFFIX "gateway"

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, mqttPort, mqttCallback, wifiClient);

//#define CHECKCONNDELTA 60     // check interval ( seconds ) for mqtt connection
//Scheduler userScheduler;   // to control your personal task

//************************************************************
// bingbong
//************************************************************


void my_setup() {



}

extern "C" void app_main(void) {
  initArduino();
  Serial.begin(115200);
  //esp_err_t ret = nvs_flash_init();

  ESP_LOGI("SETUP", "Starting CollectorMesh MQTT bridge node");

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  
  while (true) {
    mesh.update();
    mqttClient.loop();

    if(myIP != getlocalIP()){
      myIP = getlocalIP();
      Serial.println("My IP is " + myIP.toString());
      ESP_LOGI("MAIN", "My IP is %s", myIP.toString().c_str());

      if (mqttClient.connect("CollectorMeshClient","collector","esp32secret")) {
        ESP_LOGI("MAIN", "Connected to MQTT broker");
        mqttClient.publish("CollectorMesh/from/gateway","Ready!");
        mqttClient.subscribe("CollectorMesh/to/#");
    }
    else {
      ESP_LOGI("MAIN", "Failed to connect to MQTT broker");
    }
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second delay    
  }

}


void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  ESP_LOGI("DEBUG", "Received Callback");
  String topic = "CollectorMesh/from/" + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  ESP_LOGI("DEBUG", "mqtt Callback");
  char* cleanPayload = (char*)malloc(length+1);
  memcpy(cleanPayload, payload, length);
  cleanPayload[length] = '\0';
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if(targetStr == "gateway")
  {
    if(msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("CollectorMesh/from/gateway", str.c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("CollectorMesh/from/gateway", "Client not connected!");
    }
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}





