// FARM DATA RELAY SYSTEM

// Global Configuration

// Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.


#define GLOBAL_WIFI_SSID "SigmaNet"
#define GLOBAL_WIFI_PASS "Octagon1"

#define GLOBAL_MQTT_ADDR "192.168.1.13"
#define GLOBAL_MQTT_PORT 1883

//#define GLOBAL_MQTT_AUTH   //uncomment to enable MQTT authentication  
#define GLOBAL_MQTT_USER   "Your MQTT Username"
#define GLOBAL_MQTT_PASS   "Your MQTT Password"
// MQTT Topics
#define TOPIC_DATA    "fdrs/data"
#define TOPIC_STATUS  "fdrs/status"
#define TOPIC_COMMAND "fdrs/command"
#define TOPIC_DATA_BACKLOG "fdrs/databacklog"   // Used in filesystem module

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // MAC address prefix. Can be used to distinguish different ESP-NOW networks.