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
//  bubbleSort();
  mergeSort();
}

void showColour(uint32_t colour) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, colour);
  }
  strip.show();
}

void randomize(uint8_t *pixel_colour, uint16_t n) {
  uint8_t temp, pixel1, pixel2;
  uint16_t i;
  for(i=0; i<strip.numPixels(); i++) {
    pixel_colour[i] = i;
  }
  for (i=0; i<n; i++) {
    pixel1 = random(strip.numPixels());
    pixel2 = random(strip.numPixels());
    temp = pixel_colour[pixel2];
    pixel_colour[pixel2] = pixel_colour[pixel1];
    pixel_colour[pixel1] = temp;
  } 
}

void bubbleSort() {
  uint8_t pixel_colour[strip.numPixels()];
  bool swapped = true;
  uint8_t temp;
  uint16_t i;
  randomize(pixel_colour, 1000);
  while (swapped) {
    swapped = false;
    for (i=1; i<strip.numPixels(); i++) {
      if (pixel_colour[i-1] > pixel_colour[i]) {
        temp = pixel_colour[i];
        pixel_colour[i] = pixel_colour[i-1];
        pixel_colour[i-1] = temp;
        swapped = true;
        strip.setPixelColor(i-1, Wheel((pixel_colour[i-1] * 256 / strip.numPixels()) & 255));
        strip.setPixelColor(i, Wheel((pixel_colour[i] * 256 / strip.numPixels()) & 255));
        strip.show();
      }
    }
  }
}


void mergeSort() {
  uint8_t A[strip.numPixels()];
  uint8_t B[strip.numPixels()];
  bool swapped = true;
  uint8_t temp;
  uint16_t i;
  
  randomize(A, 1000);

  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((A[i] * 256 / strip.numPixels()) & 255));
    strip.show();
  }

  TopDownSplitMerge(A, 0, strip.numPixels(), B);

  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((A[i] * 256 / strip.numPixels()) & 255));
    strip.show();
  }
}

// iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set)
void TopDownSplitMerge(uint8_t *A, uint16_t iBegin, uint16_t iEnd, uint8_t *B)
{
    if (iEnd - iBegin < 2) {
        return;
    }
    // recursively split runs into two halves until run size == 1,
    // then merge them and return back up the call chain
    uint16_t iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
    TopDownSplitMerge(A, iBegin,  iMiddle, B);  // split / merge left  half
    TopDownSplitMerge(A, iMiddle,    iEnd, B);  // split / merge right half
    TopDownMerge(A, iBegin, iMiddle, iEnd, B);  // merge the two half runs
    CopyArray(B, iBegin, iEnd, A);              // copy the merged runs back to A
}

//  left half is A[iBegin :iMiddle-1]
// right half is A[iMiddle:iEnd-1   ]
void TopDownMerge(uint8_t *A, uint16_t iBegin, uint16_t iMiddle, uint16_t iEnd, uint8_t *B)
{
    uint16_t i0 = iBegin, i1 = iMiddle;
    
    // While there are elements in the left or right runs
    for (uint16_t j = iBegin; j < iEnd; j++) {
        // If left run head exists and is <= existing right run head.
        if (i0 < iMiddle && (i1 >= iEnd || A[i0] <= A[i1])) {
            B[j] = A[i0];
            i0 = i0 + 1;
        } else {
            B[j] = A[i1];
            i1 = i1 + 1;    
        }
    } 
}

void CopyArray(uint8_t *B, uint16_t iBegin, uint16_t iEnd, uint8_t *A)
{
    for(uint8_t k = iBegin; k < iEnd; k++) {
        A[k] = B[k];
        strip.setPixelColor(k, Wheel((A[k] * 256 / strip.numPixels()) & 255));
        strip.show();
        delay(5);
    }
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
