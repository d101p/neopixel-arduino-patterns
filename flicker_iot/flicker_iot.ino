/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

/************************* NeoPixel Setup *********************************/

#define PIN D3

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "happypanda2G"
#define WLAN_PASS       "0829907099"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "d101p"
#define AIO_KEY         "076a5bcdc4894a588a12ae8ba715c197"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'colour' for subscribing to changes.
Adafruit_MQTT_Subscribe colour = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lampcolour");

/*************************** NeoPixel Setup ************************************/

Adafruit_NeoPixel strip = Adafruit_NeoPixel(180, PIN, NEO_GRB + NEO_KHZ800);

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for colour feed.
  mqtt.subscribe(&colour);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'  
}

uint32_t x=0;


void loop() {
  uint16_t i;
  static uint16_t col;
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(200))) {
    if (subscription == &colour) {
//      Serial.print(F("Got: "));
//      Serial.println((char *)colour.lastread);
      col = atoi((char *)colour.lastread);
    }
  }
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
//  if(! mqtt.ping()) {
//    mqtt.disconnect();
//  }

  flicker(70, col);
}

void flicker(uint8_t wait, uint16_t col) {
  uint8_t pixel_state[strip.numPixels()];
  uint8_t time_on[strip.numPixels()];
  uint16_t i;
  uint32_t colour;
  uint8_t fading_white;

  for(i=0; i<strip.numPixels(); i++) {
    if (random(100)<5) {
      pixel_state[i] = 0;
    } else {
      pixel_state[i] += 1;
    }
  }

  for(i=0; i<strip.numPixels(); i++) {
    if (pixel_state[i]==0) {
//      colour = Wheel(((i) * 256 / strip.numPixels()) & 255);
      colour = Wheel(col);
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
//  delay(wait);

}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

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

