// --- LIBRARIES INCLUDE ---
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WEMOS_SHT3X.h>

// --- DEFAULT SETTINGS ---
#define ESP_ID          "1/1"

#define WIFI_SSID           "SSID"
#define WIFI_PASSWORD       "PASS"

#define MQTT_SERVER         "192.168.10.1"

#define MQTT_TOPIC_TEMPERATURE  "sensor/1-1/temperature" //sensor/ID/temperature - ID room: floor-room
#define MQTT_TOPIC_HUMIDITY     "sensor/1-1/humidity"

#define PUBLISH_RATE        3      // int seconds
#define SLEEP_MODE          false       // deep sleep then reset or idling (D0 must be connected to RST) https://github.com/nodemcu/nodemcu-devkit-v1.0

#define DEBUG               true      // debug to serial port

// Temperature 
float t;
float h;

// --- LIBRARIES INIT ---
WiFiClient    wifi;
PubSubClient client(wifi);
SHT3X         sht30(0x45);


void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi:");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected to");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a client ID
    String clientId = "ESP-SENSOR-ID-";
    clientId += ESP_ID;
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // after connect
      //client.publish("outTopic", "hello world");
      //client.publish("inTopic", "intopic/22");
      // ... and resubscribe
      //client.subscribe("sensor/1-1/temperature");
    } else {
      Serial.print("Error! Message: ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  if (DEBUG) Serial.begin(9600);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
}

void loop() {

  // Reading SHT30
  sht30.get();
  float t = sht30.cTemp;
  float h = sht30.humidity;

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish to MQTT
  client.publish(MQTT_TOPIC_TEMPERATURE,  String(t).c_str(), true);
  client.publish(MQTT_TOPIC_HUMIDITY, String(h).c_str(), true);


  //DeepSleep mode
  if (SLEEP_MODE) {
    if (DEBUG) {
      Serial.print("Sleeping ");
      Serial.print(PUBLISH_RATE);
      Serial.println(" seconds...");
    }
    ESP.deepSleep(PUBLISH_RATE * 1e6); //translate seconds to us
  } else {
    if (DEBUG) {
      Serial.print("Wating ");
      Serial.print(PUBLISH_RATE);
      Serial.println(" seconds...");
    }
    delay(PUBLISH_RATE * 1e3); //translate seconds to ms
  }
}
