#include <APA102.h>
#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"

// Declare command flags
bool chopit = 0; 
bool crushit = 0; 
bool cookit = 1; 

// Pressure sensor 1 pin variable 
int press_1 = 45; 

// LED pin variables 
const uint8_t dataPin = 47;
const uint8_t clockPin = 48;

// Hall effect sensor variables 
int hall_1 = 46; 

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

// Create new IMU object 
BMI270 imu;

// I2C address selection 
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68

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

  // Set up for IMU
  Wire.begin(); 

  // Check if sensor is connected and initialize
  while(imu.beginI2C(i2cAddress) != BMI2_OK)
  {
      // Not connected, inform user
      //Serial.println("Error: BMI270 not connected, check wiring and I2C address!");

      // Wait a bit to see if connection is established
      delay(1000);
  }
  //Serial.println("BMI270 connected!");

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
  //Serial.println("Added to Queue");
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

    Serial.println("numbottom = "); 
    Serial.println(numbottom); 
    Serial.println("botk = "); 
    Serial.println(botk); 
    Serial.println("ledCount = "); 
    Serial.println(ledCount);
    Serial.println("currcount = "); 
    Serial.println(currCount); 
  }
  for (int i = numbottom; i < ledCount - 1; i++) {
    master[i] = master[i + 1];
  }

  master[ledCount - 1].blue = 0;  // Reset master[i + 1] which is last position unaccounted for in the for loop

  ledStrip.write(master, ledCount, bright5bit);
}

void removeBottomLED() {
  uint8_t bright5bit = bright();

  // shift everything DOWN (toward index 0)
  for (int i = 0; i < ledCount - 1; i++) {
    master[i] = master[i + 1];
  }

  // clear the top after shifting
  master[ledCount - 1].blue = 0;

  // update LED strip
  ledStrip.write(master, ledCount, bright5bit);
}

void clearBoard(){
  uint8_t bright5bit = bright();
  for(int i=0; i<ledCount-1; i++){
    master[i].blue = 0; 
  }
  master[ledCount-1].blue = 0; 

  // reset counters 
  botk = 0; 
  numbottom = 0; 
  currCount = 0; 
  
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

void startFallingLEDs(){
  // Tetris is full - ran out of time
  if (currCount == ledCount){ 
    //Serial.println("1"); 
    GameOver(); 
  }
  // Queue Timer expired - add another LED to Queue
  if (logtimeT + tetristimer <= millis()) {
    //Serial.println("2"); 
    logtimeT = millis();
    //Serial.println(tetristimer);
    ShiftStack();
    AddToQueue();
    tetristimer = tetristimer - 50; // Make turns shorter
    if (tetristimer < 100) { tetristimer = 100; }
  }
  if (logtimeF + falltimer <= millis()) {
    //Serial.println("3"); 
    logtimeF = millis();
    //Serial.println(falltimer);
    ShiftStack();
    falltimer = falltimer - 50;
    if (falltimer < 100) { falltimer = 100; }
  }
}

void loop() {
  startFallingLEDs(); 
  currentTime = millis();

  // Read sensor states 
  int press_1_state = digitalRead(press_1);
  int hall_1_state = digitalRead(hall_1); 
  imu.getSensorData(); 

  // Check if bottom LED is on
  if(master[0].blue == 255){
    botLEDOn = 1; 
  }
  else{
    botLEDOn = 0; 
  }

  // Chop-it 
  if ((chopit) && (botLEDOn) && (press_1_state == HIGH) && (currentTime - lastPressedTime >= debounceDelay)) {
    pressed = 1;
    lastPressedTime = currentTime;
  } 
  if (pressed && !crushed && !tilted) {
    incrementScore();
    removeBottomLED(); 
    pressed = 0;  
  }

  // Crush-it 
  if((crushit) && (botLEDOn) && (hall_1_state == HIGH) && (currentTime - lastPressedTime >= debounceDelay)){
    crushed = 1; 
    lastPressedTime = currentTime;
  }

  if(crushed && !pressed && !tilted){
    incrementScore(); 
    removeBottomLED(); 
    crushed = 0; 
  }
  
  // Cook-it 
  if((cookit) && (botLEDOn) && (imu.data.gyroY > 220) && (currentTime - lastPressedTime >= debounceDelay)){
    tilted = 1; 
    lastPressedTime = currentTime;
  }

  if(tilted && !pressed && !crushed){
    incrementScore(); 
    clearBoard(); 
    tilted = 0; 
  }
}
