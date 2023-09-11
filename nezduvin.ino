// Aroma picker for the Le Nez du Vin game
//
// Copyright (c) 2023 John Graham-Cumming

// Adafruit Pro Trinket 5V pin connections
//
// Category     Button     LED
// --------     ------     ---
// Fruity       PC0/14     PD6/6
// Floral       PC1/15     PD5/5
// Vegetal      PC2/16     PD4/4
// Animal       PC3/17     PD3/3
// Toasty       PC4/18     PD1/1
// Wildcard     PC6/19     PD0/0
//
// Seven-segment display connected via SPI (pins 10, 11, 13)

#include <SPI.h>

int buttons[6] = {14, 15, 16, 17, 18, 19};
int leds[6]    = {6, 5, 4, 3, 1, 0};

int range_starts[6] = { 1, 24, 30, 45, 48,  1};
int range_ends[6]   = {23, 29, 44, 47, 54, 54};

// Seed the random number generator with a combination of noise from 
// the two unconnected analog and when the first key is pressed the number
// of milliseconds elapsed since boot.

int seed = analogRead(A6) * analogRead(A7);
unsigned long start;
bool first_press = true;

void setup() {

  // I don't trust the SparkFun seven-segment display to operate at anything
  // but the lowest SPI speeds.
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);

  // Set brightness of the display
  
  spi(0x7A);
  spi(0xFE);
  clear7();

  // The buttons need the internal pullup enabled as they are set to pull
  // down when pressed.
    
  for (int i = 0; i < 6; i++) {
    pinMode(buttons[i], INPUT_PULLUP);

    pinMode(leds[i], OUTPUT);
    led(i, true);
    ss(true);
    bool sig = false;
    int num = range_starts[i] * 100 + range_ends[i];
    for (int d = 1000; d > 0; num %= d, d /= 10) {
      int digit = num / d;
      if ((digit == 0) && (!sig)) {
        spi('x');
      } else {
        spi(digit);
        sig = true;
      }
    }    

    // Turns on the : in the middle of the seven-segment module
    
    spi(0x77);
    spi(0x10);
    ss(false);
    delay(500);
    led(i, false);
  }

  start = millis();

  clear7();
  send7("pLay");
}

void loop() {
  for (int i = 0; i < 6; i++) {
    int state = digitalRead(buttons[i]);
    
    if (state == LOW) {
      pressed(i);
      return;
    }
  }
}

// pressed handles when a button is pressed and generates and displays
// a random number for the category corresponding to the button.
void pressed(int b) {
  clear7();

  // On the first press use a combination of noise from A6 and A7 and 
  // the time between the program start and now to seed the random number
  // generator.
  
  if (first_press) {
    int delta = millis() - start;
    seed *= delta;
    randomSeed(seed);
  }

  // Light the LEDs in turn to give a spinning effect that makes it look
  // like the device is thinking. Start on the button that was 
  // pressed.

  // The loop generates random numbers as it goes and displays them to 
  // make it look interesting.
  
  for (int j = 0; j < 10; j++) {
    for (int i = b; i < b+6; i++) {
      led(i % 6, true);
      delay(75);
      led(i % 6, false);

      int num = random(range_starts[b], range_ends[b]+1);
      bool sig = false;
      ss(true);
      spi('x');
      for (int d = 10; d > 0; num %= d, d /= 10) {
        int digit = num / d;
        if ((digit == 0) && (!sig)) {
          spi('x');
        } else {
          spi(digit);
          sig = true;
        }
      }    
      spi('x');
      ss(false);
    }
  }
  led(b, true);

  // Keep the chosen number on the display for a number of seconds and 
  // then turn off the display and the associated LED, and display the
  // word PLAY.
  
  delay(5000);
  led(b, false);
  clear7();
  send7("pLay");
}

// led turns on or off an LED
void led(int b, bool on) {
  digitalWrite(leds[b], on?HIGH:LOW);
}

// clear7 clears the seven-segment display
void clear7() {
  ss(true);
  spi('v');

  // This clears all the dots and the colon
  
  spi(0x77);
  spi(0x00);
  ss(false);
}

// send7 writes a string to the seven-segment display
void send7(char *s) {
  ss(true);
  for (; *s != '\0'; s++) {
    spi(*s);
  }
  ss(false);
}

// spi sends a byte to the seven-segment display
void spi(byte b) {
  SPI.transfer(b);
}

// ss selects or deselects the seven-segment displays SPI interface
void ss(bool enable) {
  digitalWrite(SS, enable?LOW:HIGH);
}
