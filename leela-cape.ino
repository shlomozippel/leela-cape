#include <FastLED.h>

#define NUM_LEDS 32
#define LED_PIN 3
#define SWITCH_PIN 11
#define LONG_PRESS_TIME 1000

#define FRAMES_PER_SECOND 60
#define BRIGHTNESS 190
#define RAINBOW_SPEED 7

uint8_t color = 0;
CRGB leds[NUM_LEDS];

// rotary encoder state
bool g_pressed = false;
bool g_pressedThisFrame = false;
bool g_releasedThisFrame = false;
bool g_longPress = false;
bool g_longPressThisFrame = false;
uint16_t g_pressTime = 0;
uint16_t g_releaseTime = 0;
// volatile because it'll be updated from ISR
volatile int16_t g_rotenc = 200;

// current timestamp
uint16_t g_now = 0;


void setup() { 

    init_rotenc_pins();    
    pinMode(SWITCH_PIN, INPUT_PULLUP); // Switch

    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
}

void loop() { 
    g_now = millis();
    read_switch();

    uint16_t delta = g_now - g_pressTime;
    if (delta <= 1000) {
      FastLED.setBrightness(map(delta, 0, 1000, 0, BRIGHTNESS));      
    }

    fill_rainbow( leds, NUM_LEDS, color + g_rotenc, RAINBOW_SPEED);
    FastLED.show();
    //color++;
    FastLED.delay(1000/FRAMES_PER_SECOND);
}

void init_rotenc_pins(){
  // set pins to input
  pinMode(12, INPUT); // A - PB4
  pinMode(13, INPUT); // B - PB5
  
  // setup pin change interrupt
  cli();  
  PCICR = 0x01;
  PCMSK0 = 0b00110000;
  sei();
}

int8_t read_encoder()
{
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  
  old_AB <<= 2;                   
  old_AB |= (( PINB & 0b00110000 ) >> 4);
  return ( enc_states[( old_AB & 0x0f )]);
}

ISR(PCINT0_vect) {
  g_rotenc -= read_encoder();
/*  if (g_rotenc < 0) {
    g_rotenc = 0; 
  }
  if (g_rotenc > 255) {
    g_rotenc = 255;
  }*/
}

void read_switch() {
    g_releasedThisFrame = false;
    g_pressedThisFrame = false;
    g_longPressThisFrame = false;
    
    bool currentPressed = digitalRead(SWITCH_PIN) == LOW;
    if (currentPressed != g_pressed) {
      // release
      if (g_pressed) {
        g_releaseTime = g_now;
        g_pressed = false;
        g_releasedThisFrame = true;
      // press
      } else {
        // simple debounce
        g_longPress = false;
        if (g_now - g_releaseTime > 50) {
          g_pressTime = g_now;
          g_pressed = true;
          g_pressedThisFrame = true;
        }
      }
    }
    else if (g_pressed && g_now - g_pressTime > LONG_PRESS_TIME) {
       g_longPress = true;
       g_longPressThisFrame = true;
    }
}

