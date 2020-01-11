#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Adafruit_NeoPixel.h>

#define PIN D3

#define WIFI_SSID "happypanda2G"
#define WIFI_PASSWORD "0829907099"

#define MQTT_HOST       "io.adafruit.com"
#define MQTT_PORT       1883
#define MQTT_USERNAME    "d101p"
#define MQTT_KEY         "076a5bcdc4894a588a12ae8ba715c197"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(180, PIN, NEO_GRB + NEO_KHZ800);

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

uint16_t colour = 20;
uint16_t lampspeed = 70;
uint16_t flicker_colour = 5;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  uint16_t packetIdSub;
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  packetIdSub = mqttClient.subscribe("d101p/feeds/lampcolour", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  packetIdSub = mqttClient.subscribe("d101p/feeds/lampspeed", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  packetIdSub = mqttClient.subscribe("d101p/feeds/lampflicker", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  uint16_t packetIdPub1 = mqttClient.publish("d101p/feeds/lampcolour", 1, true, "20");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("d101p/feeds/lampspeed", 2, true, "70");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
  uint16_t packetIdPub3 = mqttClient.publish("d101p/feeds/lampflicker", 2, true, "5");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub3);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  if (strcmp(topic, "d101p/feeds/lampcolour") == 0) {
    colour = atoi((char *)payload);
    Serial.println(colour);
  } else if (strcmp(topic, "d101p/feeds/lampspeed") == 0) {
    lampspeed = atoi((char *)payload);
    Serial.println(lampspeed);
  } else if (strcmp(topic, "d101p/feeds/lampflicker") == 0) {
    flicker_colour = atoi((char *)payload);
    Serial.println(flicker_colour);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, "076a5bcdc4894a588a12ae8ba715c197");

  connectToWifi();

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  randomSeed(analogRead(0));
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
}

void loop() {
  ArduinoOTA.handle();
  flicker(lampspeed);
}

void flicker(uint8_t wait) {
  static uint32_t count = 0;
  uint8_t pixel_state[strip.numPixels()];
  uint8_t time_on[strip.numPixels()];
  uint16_t i;
  uint32_t colour;
  uint8_t fading_white;

  for(i=0; i<strip.numPixels(); i++) {
    if (random(100)<flicker_colour) {
      pixel_state[i] = 0;
    } else {
      pixel_state[i] += 1;
    }
  }

  for(i=0; i<strip.numPixels(); i++) {
    if (pixel_state[i]==0) {
      colour = Wheel((((i) * 256 / strip.numPixels()) + count) & 255);
    } else {
      fading_white = 60-10*pixel_state[i];
      if (60-10*pixel_state[i] < 5) {
       if (60-10*pixel_state[i] < -5) {
          fading_white = 5;
        } 
        else
        {
          fading_white = 120;
        }
      } 
      colour = strip.Color(fading_white, fading_white, fading_white);
    }
    strip.setPixelColor(i, colour);
  }
  strip.show();
  delay(wait);
  count++;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

