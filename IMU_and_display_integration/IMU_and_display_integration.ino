#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"

// Create a new IMU object
BMI270 imu;

// Declare IMU flags
bool tilted = 0; 
unsigned long getTime = 0; 

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

// I2C address selection
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68
//uint8_t i2cAddress = BMI2_I2C_SEC_ADDR; // 0x69

void setup()
{
    // Configure 14 segment display pins 
    for (int i = 0; i < 8; i++) {
    pinMode(ScoreDisplay[i], OUTPUT);
    }

    // Start serial
    Serial.begin(115200);

    // Initialize the I2C library
    Wire.begin();

    // Check if sensor is connected and initialize
    while(imu.beginI2C(i2cAddress) != BMI2_OK)
    {
        // Not connected, inform user
        Serial.println("Error: BMI270 not connected, check wiring and I2C address!");

        // Wait a bit to see if connection is established
        delay(1000);
    }
    Serial.println("BMI270 connected!");
}

// Function to increment current score 
void incrementScore() {
  currScore++;
  for (int i = 0; i < 8; i++) {
    bool bit = (currScore >> i) & 1;
    digitalWrite(ScoreDisplay[i], bit ? HIGH : LOW);
  }
}


void loop(){
  
  if(!tilted){
    imu.getSensorData(); 
    //Serial.println(1); 

    if(imu.data.gyroY > 220){
      tilted = 1; 
      incrementScore(); 
      //Serial.println(2); 

      getTime = millis();
      
      while(millis() < getTime + 10000){
        tilted = 1; 
        //Serial.print(3); 
      }

      tilted = 0; 
      //Serial.print(4); 
    }
  }
}

    