#include <FastLED.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"

 
#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif
//globalPattern


// ULTRASONIC SENSOR
int TRIG = 3;
int ECHO = 2;
int DURATION;
int DISTANCE;
// hoisted function name to create name before definition below.
void distance_loop();


// TOUCH SENSOR
// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
 
// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
void touch_loop();


// LED STUFF from Example DemoReel100
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    13 // next to ground
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    64 // update to number of leds in the strip
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))


// Hoisted pattern function list. 
// this is defining the names before declaring the logic below so it can be used early in the script.

void rainbow();
void rainbowWithGlitter();
void confetti();
void sinelon();
void juggle();
void bpm();

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
uint8_t patternCount = 5;
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns


 
 
void setup() {  
 
  // ULTRASONIC SENSOR SETUP
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }

  // TOUCH SENSOR SETUP
  //Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  delay(100);

  // LED SETUP
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}
 
// THESE LOOPS ARE COPIED FROM THE RESPECTIVE EXAMPLE CODE PROVIDED BUT WITH DELAY REMOVED
void distance_loop() {
 
  digitalWrite(TRIG,HIGH);
  delay(1);
  digitalWrite(TRIG,LOW);
  DURATION = pulseIn(ECHO,HIGH);
  DISTANCE = DURATION / 58.2;
 
  if(DISTANCE > 0 && DISTANCE < 50 ){
    Serial.print("DISTANCE:");
    Serial.println(DISTANCE);
    //delay(100); this is handled by timer
  }
}
 
void touch_loop() {
  // Get the currently touched pads
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print("TOUCH:");
      Serial.println(i); 
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print("RELEASE:");
      Serial.println(i); 
    }
  }
 
  // reset our state
  lasttouched = currtouched;
 
  // comment out this line for detailed data from the sensor!
  return;
  
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  
  // put a delay so it isn't overwhelming
  //delay(100); this is handled by touch timer
}


// READ SERIAL FROM TD
// EXAMPLE FROM ReadASCIIString from Arduino Communication Examples
// This expects just a integer per line from TD
// It should be within the range of the LED Pattern count - 1 (starts with 0)
void read_seriol_loop() {
  // if there's any serial available, read it:
  while (Serial.available() > 0) {

    // look for the next valid integer in the incoming serial stream:
    int pattern_number = Serial.parseInt();

    // look for the newline. That's the end of your line:
    if (Serial.read() == '\n') {
      // constrain the values to 0 -6 wihc is the number of patterns in gPatterns
      gCurrentPatternNumber = constrain(pattern_number, 0, patternCount);

    }
  }
}



void loop(){
  // This runs in a loop and does each of the EVERY_N only periodically insteaf of every loop, defined by the num in the parens.
 EVERY_N_MILLISECONDS(75) {touch_loop();}
 EVERY_N_MILLISECONDS(100){distance_loop();}
 EVERY_N_MILLISECONDS(30) {read_seriol_loop();}


 // This is the LED stuff from DemoReel100 and runs much more often and defines the framerate of the LED Updates
 
 // instead of naming one of the patterns, we just use the pattern number to run the whichever pattern we want from the list.
 gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  //EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically. instead we are reading the pattern number from serial

}


// DEMO REEL 100 PATTERNS  the order is defined above in the gPatterns list
void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
