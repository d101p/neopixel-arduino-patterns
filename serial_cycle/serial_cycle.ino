#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(180, PIN, NEO_GRB + NEO_KHZ800);

char inputString[10];         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(9600);
}

void loop() {
  byte input;

  serialEvent();
  if (stringComplete) {
    uint32_t colour = strtoul(inputString, 0, 16);
    colorWipe(colour);
//    Serial.println(inputString);
    
    stringComplete = false;
  }

}

void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
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

void serialEvent() {
  int char_pos = 0;
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString[char_pos] = inChar;
    if (inChar == '\n') {
      inputString[char_pos] = '\0';
      stringComplete = true;
    }
    char_pos += 1;
  }
  Serial.print("ACK: ");
  Serial.println(inputString);
}
