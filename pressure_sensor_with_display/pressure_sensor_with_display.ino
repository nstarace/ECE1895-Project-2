#include <APA102.h>
#include <Arduino.h>

// Declare command flags
bool chopit = 0; 
bool crushit = 1; 
bool cookit = 0; 

// Pressure sensor 1 pin variable 
int press_1 = 45; 

// LED pin variables 
const uint8_t dataPin = 47;
const uint8_t clockPin = 48;

// Hall effect sensor variables 
int hall_1 = 46; 

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

// Declare 14 segment display variables 
uint64_t sample = 0;
const uint8_t a1 = 41;
const uint8_t b1 = 1;
const uint8_t c1 = 2;
const uint8_t d1 = 42;
const uint8_t a2 = 37;
const uint8_t b2 = 40;
const uint8_t c2 = 39;
const uint8_t d2 = 38;
const uint8_t ScoreDisplay[8] = {a2, b2, c2, d2, a1, b1, c1, d1};

// Declare current score variable 
byte currScore = 0x00;

// Declare LED variables 
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
//bool pressed = 0;
//byte currScore = 0x00;
int strikes = 0;
int numbottom = 0;
bool addsingle = 1;
int botk = 0;

rgb_color master[allLED];

// Declare sensor variables
unsigned long currentTime = 0;
unsigned long lastPressedTime = 0;
const unsigned long debounceDelay = 5000; // 5 seconds
bool botLEDOn = 0; 
bool pressed = 0; 
bool crushed = 0; 
bool tilted = 0; 


void setup(){
  // Start serial
  Serial.begin(115200); 

  // Configure 14 segment display pins 
  for (int i = 0; i < 8; i++) {
    pinMode(ScoreDisplay[i], OUTPUT);
  }

  // Set pin on ESP32 for pressure sensor 1
  pinMode(press_1,INPUT); 

  // Set pin on ESP32 for hall effect sensor
  pinMode(hall_1, INPUT); 

  // Set up for LEDs
  pinMode(ButtonPressPin, INPUT); // Passive pulldown
  uint8_t bright5bit = bright();
  for (int i = 0; i < allLED; i++) {
    master[i].blue = 0;
    master[i].red = 0;
    master[i].green = 0;
  }
  ledStrip.write(master, allLED, bright5bit);
}

// Function to increment current score 
void incrementScore() {
  currScore++;
  for (int i = 0; i < 8; i++) {
    bool bit = (currScore >> i) & 1;
    digitalWrite(ScoreDisplay[i], bit ? HIGH : LOW);
  }
}

// Function to adjust LED brightness 
uint8_t bright() {
  uint8_t bright5bit = 1;
  while (bright5bit * 255 < power && bright5bit < 31) { bright5bit++; }
  uint8_t intensity = (power + (bright5bit / 2)) / bright5bit;
  return bright5bit;
}

// Function to add LED to queue 
void AddToQueue() {
  Serial.println("Added to Queue");
  uint8_t bright5bit = bright();
  master[ledCount - 1].blue = 255;
  ledStrip.write(master, ledCount, bright5bit);
  currCount = currCount + 1;
}

// Function to shift LED stack 
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

// Function for LED game-over sequence 
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
  
  // Increment score if pressed and LED is at the bottom 
  currentTime = millis();

  // Read sensor states 
  int press_1_state = digitalRead(press_1);
  int hall_1_state = digitalRead(hall_1); 

  // Check if bottom LED is on
  if(master[0].blue == 255){
    botLEDOn = 1; 
  }
  else{
    botLEDOn = 0; 
  }

  // Chop-it (also need to add not crushed and not cooked)
  if (chopit && botLEDOn && press_1_state == HIGH && (currentTime - lastPressedTime >= debounceDelay)) {
    pressed = 1;
    lastPressedTime = currentTime;
  } 
  if (pressed) {
    incrementScore();
    pressed = 0;  
  }

  // Crush-it 
  if(crushit && botLEDOn && hall_1_state == HIGH && (currentTime - lastPressedTime >= debounceDelay)){
    crushed = 1; 
    lastPressedTime = currentTime;

    if(crushed){
      incrementScore(); 
      crushed = 0; 
    }
  }
}
