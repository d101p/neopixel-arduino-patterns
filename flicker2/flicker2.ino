#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(180, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  randomSeed(analogRead(0));
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Some example procedures showing how to display to the pixels:
  flicker(70);
}

// Fill the dots one after the other with a color
void flicker(uint8_t wait) {
  bool pixel_state[strip.numPixels()];
  uint8_t time_on[strip.numPixels()];
  uint16_t i;
  uint32_t colour;

  for(i=0; i<strip.numPixels(); i++) {
    if (random(100)<50) {
      pixel_state[i] = true;
    } else {
      pixel_state[i] = false;
    }
  }

  for(i=0; i<strip.numPixels(); i++) {
    if (pixel_state[i]) {
      colour = Wheel(((i) * 256 / strip.numPixels()) & 255);
    } else {
      colour = strip.Color(0, 0, 0);
    }
    strip.setPixelColor(i, colour);
  }
  strip.show();
  delay(wait);
  
//  while(true){
//    for(i=0; i<strip.numPixels(); i++) {
//      if(i==pos) {
//        colour = Wheel((i * 256 / strip.numPixels()) & 255);
//      } else {
//        colour = strip.Color(0, 0, 0);
//      }
//      strip.setPixelColor(i, colour);
//    }
//    strip.show();
//    delay(wait);
//  }
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
