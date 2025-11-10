/* This example shows how to make an LED pattern with a large
 * dynamic range using the the extra 5-bit brightness register in
 * the APA102.
 *
 * It sets every LED on the strip to white, with the dimmest
 * possible white at the input end of the strip and the brightest
 * possible white at the other end, and a smooth logarithmic
 * gradient between them.
 *
 * The dimmest possible white is achieved by setting the red,
 * green, and blue color channels to 1, and setting the
 * brightness register to 1.  The brightest possibe white is
 * achieved by setting the color channels to 255 and setting the
 * brightness register to 31.
 */

/* By default, the APA102 library uses pinMode and digitalWrite
 * to write to the LEDs, which works on all Arduino-compatible
 * boards but might be slow.  If you have a board supported by
 * the FastGPIO library and want faster LED updates, then install
 * the FastGPIO library and uncomment the next two lines: */
// #include <FastGPIO.h>
// #define APA102_USE_FAST_GPIO

#include <APA102.h>
#include <Arduino.h>

const uint8_t dataPin = 47;
const uint8_t clockPin = 48;

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

const uint16_t allLED = 60;
const uint16_t ledCount = 10;
const uint16_t maxPower = 255 * 31;
const uint16_t minPower = 1;
const uint8_t power = 25; //  CHANGE THIS FOR BRIGHTNESS
const float multiplier = pow(maxPower / minPower, 1.0 / (ledCount - 1)); // 1.1643
double tetristimer = 4000; // in ms and speeds up -> time between adding LEDs
double falltimer = 2000; // in ms and speeds up -> time between falling LEDs
double logtimeT = 0;
double logtimeF = 0;
bool gameover = 0;
int16_t currCount = 0;
const uint8_t ButtonPressPin = 35; // ******SET PINNNN******************************
bool pressed = 0;
byte currScore = 0x00;
int strikes = 0;
int numbottom = 0;
bool addsingle = 1;
int botk = 0;

rgb_color master[allLED];

//const float power = minPower * pow(multiplier, 10);

void setup() {
  Serial.begin(115200);
  pinMode(ButtonPressPin, INPUT); // Passive pulldown
  uint8_t bright5bit = bright();
  for (int i = 0; i < allLED; i++) {
    master[i].blue = 0;
    master[i].red = 0;
    master[i].green = 0;
  }
  ledStrip.write(master, allLED, bright5bit);
}

uint8_t bright() {
  uint8_t bright5bit = 1;
  while (bright5bit * 255 < power && bright5bit < 31) { bright5bit++; }
  uint8_t intensity = (power + (bright5bit / 2)) / bright5bit;
  return bright5bit;
}

void AddToQueue() {
  Serial.println("Added to Queue");
  uint8_t bright5bit = bright();
  master[ledCount - 1].blue = 255;
  ledStrip.write(master, ledCount, bright5bit);
  currCount = currCount + 1;
}

void ShiftStack() {
  //Serial.println("Stack Shifted");
  uint8_t bright5bit = bright();
  if (master[botk].blue == 255) {
    numbottom = numbottom + 1;
    botk = botk + 1;
  }
  for (int i = numbottom; i < ledCount - 1; i++) {
    master[i] = master[i + 1];
  }

  master[ledCount - 1].blue = 0;  // Reset master[i + 1] which is last position unaccounted for in the for loop

  ledStrip.write(master, ledCount, bright5bit);
}

void GameOver() {
  uint8_t bright5bit = bright();
  // Game Over Sequence
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < ledCount; j++) {
      master[j].blue = 0;
      master[j].red = 255;
    }
    ledStrip.write(master, ledCount, bright5bit);
    delay(500);
    for (int j = 0; j < ledCount; j++) {
      master[j].red = 0;
    }
    ledStrip.write(master, ledCount, bright5bit);
    delay(500);
  }
}

void loop() {
  // Tetris is full - ran out of time
  if (currCount == ledCount) { GameOver(); }
  
  // Queue Timer expired - add another LED to Queue
  if (logtimeT + tetristimer <= millis()) {
    logtimeT = millis();
    Serial.println(tetristimer);
    ShiftStack();
    AddToQueue();
    tetristimer = tetristimer - 50; // Make turns shorter
    if (tetristimer < 100) { tetristimer = 100; }
  }

  if (logtimeF + falltimer <= millis()) {
    logtimeF = millis();
    Serial.println(falltimer);
    ShiftStack();
    falltimer = falltimer - 50;
    if (falltimer < 100) { falltimer = 100; }
  }
  /*// Read button press
  pressed = digitalRead(ButtonPressPin);  
  if (pressed) {
    if (master[0].blue == 255) {
      ShiftStack();
      currScore = currScore + 1;
      // Implement Debounce
    }
    else { // LED does not exist
      strikes = strikes + 1;
      if (strikes == 3) { GameOver(); }
    }
  }*/
}