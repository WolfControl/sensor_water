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


// PainlessMesh credentials ( name, password and port ): You should change these
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// WiFi credentials: should match your access point!
#define   STATION_SSID     "SigmaNet"
#define   STATION_PASSWORD "BINGBONG"

#define   HOSTNAME         "MQTT_Bridge"

Scheduler userScheduler;   // to control your personal task

painlessMesh  mesh;
WiFiClient wifiClient;

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();
IPAddress myIP(0,0,0,0);


// GCP broker address and port
char mqttBroker[]  = "34.135.33.166";
#define MQTTPORT 1883

// topic's suffix: everyone can publish/subscribe to this public broker,
// you have to change the following 2 defines
#define PUBPLISHSUFFIX             "painlessMesh/from/"
#define SUBSCRIBESUFFIX            "painlessMesh/to/"

#define PUBPLISHFROMGATEWAYSUFFIX  PUBPLISHSUFFIX "gateway"

#define CHECKCONNDELTA 60     // check interval ( seconds ) for mqtt connection

PubSubClient mqttClient;


bool calc_delay = false;
SimpleList<uint32_t> nodes;
uint32_t nsent=0;
char buff[512];
uint32_t nexttime=0;
uint8_t  initialized=0;


// messages received from the mqtt broker
void mqttCallback(char* topic, uint8_t* payload, unsigned int length) 
  {
  char* cleanPayload = (char*)malloc(length+1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length+1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  Serial.printf("mc t:%s  p:%s\n", topic, payload);
  
  String targetStr = String(topic).substring(strlen(SUBSCRIBESUFFIX));
  if(targetStr == "gateway")
    {
    if(msg == "getNodes")
      {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, str.c_str());
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
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, "Client not connected!");
      }
    }
  }


// messages received from painless mesh network
void receivedCallback( const uint32_t &from, const String &msg ) 
  {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = PUBPLISHSUFFIX + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
  }


void newConnectionCallback(uint32_t nodeId) 
  {
  Serial.printf("--> Start: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> Start: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
  }


void changedConnectionCallback() 
  {
  Serial.printf("Changed connections\n");

  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) 
    {
    Serial.printf(" %u", *node);
    node++;
    }
  Serial.println();
  calc_delay = true;

  sprintf(buff,"Nodes:%d",nodes.size());
  }


void nodeTimeAdjustedCallback(int32_t offset) 
  {
  Serial.printf("Adjusted time %u Offset = %d\n", mesh.getNodeTime(),offset);
  }


void onNodeDelayReceived(uint32_t nodeId, int32_t delay)
  {
  Serial.printf("Delay from node:%u delay = %d\n", nodeId,delay);
  }


void reconnect()
{
  //byte mac[6];
  char MAC[9];
  int i;

  // unique string
  //WiFi.macAddress(mac);
  //sprintf(MAC,"%02X",mac[2],mac[3],mac[4],mac[5]);
  sprintf(MAC, "%08X",(uint32_t)ESP.getEfuseMac());  // generate unique addresss.
  // Loop until we're reconnected
  while (!mqttClient.connected()) 
    {
    Serial.println("Attempting MQTT connection...");    
    // Attemp to connect
    if (mqttClient.connect(/*MQTT_CLIENT_NAME*/MAC)) 
      {
      Serial.println("Connected");  
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX,"Ready!");
      mqttClient.subscribe(SUBSCRIBESUFFIX "#");
      } 
    else
      {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
      mesh.update();
      mqttClient.loop();
      }
    }
}




void init_storage(void)
{
  const char *TAG = "STORAGE";

  ESP_LOGI("STORAGE", "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
  };

  // Use settings defined above to initialize and mount SPIFFS filesystem.
  // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
      if (ret == ESP_FAIL) {
          ESP_LOGE(TAG, "Failed to mount or format filesystem");
      } else if (ret == ESP_ERR_NOT_FOUND) {
          ESP_LOGE(TAG, "Failed to find SPIFFS partition");
      } else {
          ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
      }
      return;
  }
}

void start_mesh() 
  {

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on except GENERAL
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 1. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  // if you want your node to accept OTA firmware, simply include this line
  // with whatever role you want your hardware to be. For instance, a
  // mesh network may have a thermometer, rain detector, and bridge. Each of
  // those may require different firmware, so different roles are preferrable.
  //
  // MAKE SURE YOUR UPLOADED OTA FIRMWARE INCLUDES OTA SUPPORT OR YOU WILL LOSE
  // THE ABILITY TO UPLOAD MORE FIRMWARE OVER OTA. YOU ALSO WANT TO MAKE SURE
  // THE ROLES ARE CORRECT
  mesh.initOTAReceive("bridge");

  sprintf(buff,"Id:%d",mesh.getNodeId());
  
  mqttClient.setServer(mqttBroker, MQTTPORT);
  mqttClient.setCallback(mqttCallback);  
  mqttClient.setClient(wifiClient);
  }





extern "C" void app_main(void) 
  {
  //init_storage();

  start_mesh();
  mesh.update();

  /*
  mqttClient.loop();

  if(myIP != getlocalIP())
    {
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
    initialized = 1;
    }
  if ( ( millis() >= nexttime ) && ( initialized ) )
    {
    nexttime=millis()+CHECKCONNDELTA*1000;
    if (!mqttClient.connected()) 
      {reconnect();}
    }
  */

  }



  IPAddress getlocalIP() 
  {
  return IPAddress(mesh.getStationIP());
  }
