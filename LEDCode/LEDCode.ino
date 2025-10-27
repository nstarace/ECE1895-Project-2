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

const uint8_t dataPin = 9;
const uint8_t clockPin = 10;

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

const uint16_t ledCount = 60;
const uint16_t maxPower = 255 * 31;
const uint16_t minPower = 1;
const float multiplier = pow(maxPower / minPower, 1.0 / (ledCount - 1)); // 1.1643

rgb_color colors[ledCount];

//const float power = minPower * pow(multiplier, 10);

void setup()
{
}

void sendWhite(uint8_t power) {
  //int bright = calibrateBrightness();
  uint8_t bright5bit = 1;
  while(bright5bit * 255 < power && bright5bit < 31)
  {
    bright5bit++;
  }

  uint8_t intensity = (power + (bright5bit / 2)) / bright5bit;

  ledStrip.sendColor(intensity, intensity, intensity, bright5bit);
}

void sendRed(uint8_t power) {
  //int bright = calibrateBrightness();
  uint8_t bright5bit = 1;
  while(bright5bit * 255 < power && bright5bit < 31)
  {
    bright5bit++;
  }

  uint8_t intensity = (power + (bright5bit / 2)) / bright5bit;

  ledStrip.sendColor(intensity, 0, 0, bright5bit);
}

void sendBlue(uint8_t power) {
  //int bright = calibrateBrightness();
  uint8_t bright5bit = 1;
  while(bright5bit * 255 < power && bright5bit < 31)
  {
    bright5bit++;
  }

  uint8_t intensity = (power + (bright5bit / 2)) / bright5bit;

  ledStrip.sendColor(0, 0, intensity, bright5bit);
}

void sendGreen(uint8_t power) {
  //int bright = calibrateBrightness();
  uint8_t bright5bit = 1;
  while(bright5bit * 255 < power && bright5bit < 31)
  {
    bright5bit++;
  }

  uint8_t intensity = (power + (bright5bit / 2)) / bright5bit;

  for (int i = 0; i < ledCount; i++) {
    colors[i].red = 0;
    colors[i].green = 255;
    colors[i].blue = 0;
  }

  ledStrip.write(colors, ledCount, bright5bit);
}

void loop() {
  bool done = 0;
  static int maxLED = 10;
  static int currCount = -1;
  static int currIndex = maxLED;

  float power = 25;
  uint8_t bright5bit = 1;
  while(bright5bit * 255 < power && bright5bit < 31)
  {
    bright5bit++;
  }
  while (!done) {
    while (currIndex > currCount) {
      colors[currIndex].blue = 255;
      ledStrip.write(colors, ledCount, bright5bit);
      delay(1000);
      currIndex = currIndex - 1;
      if (currIndex == currCount) { 
        currCount = currCount + 1;
        currIndex = maxLED;
        break; 
      }
      colors[currIndex + 1].blue = 0;
      ledStrip.write(colors, ledCount, bright5bit);
    }
    if (currIndex == currCount) { done = 1; }
  }

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j <= maxLED; j++) {
      colors[j].blue = 0;
      colors[j].red = 255;
    }
    ledStrip.write(colors, ledCount, bright5bit);
    delay(500);
    for (int j = 0; j <= maxLED; j++) {
      colors[j].red = 0;
    }
    ledStrip.write(colors, ledCount, bright5bit);
    delay(500);
  }

  /*ledStrip.startFrame();
  float power = 25;
  for(uint16_t i = 0; i < ledCount; i++)
  {
    sendGreen(power);
  }
  ledStrip.endFrame(ledCount);*/
}