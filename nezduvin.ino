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
    
    // Turns on the : in the middle of the seven-segment module

    ss(true);
    spi(0x77);
    spi(0x10);
    ss(false);
    number7(range_starts[i] * 100 + range_ends[i]);

    delay(500);
    led(i, false);
  }

  start = millis();

  play7();
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
      number7(random(range_starts[b], range_ends[b]+1));
    }
  }
  led(b, true);

  // Keep the chosen number on the display for a number of seconds and 
  // then turn off the display and the associated LED, and display the
  // word PLAY
  
  delay(5000);
  led(b, false);
  play7();
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

// number7 writes a number (up to 4 digits) to the seven-segment
// display. If it's two digits then it's centered
void number7(int n) {
  bool two = (n < 100);

  ss(true);

  if (two) {
    spi('x');
  }
  
  bool sig = false;
  for (int d = two?10:1000; d > 0; n %= d, d /= 10) {
    int digit = n / d;
  
    if ((digit == 0) && (!sig)) {
      spi('x');
    } else {
      spi(digit);
      sig = true;
    }
  }
  
  if (two) {
    spi('x');
  }

  ss(false);
}

// play7 shows the word PLAY on the seven-segment display
void play7() {
  clear7();
  send7("pLay");
}

// spi sends a byte to the seven-segment display
void spi(byte b) {
  SPI.transfer(b);
}

// ss selects or deselects the seven-segment displays SPI interface
void ss(bool enable) {
  digitalWrite(SS, enable?LOW:HIGH);
}
