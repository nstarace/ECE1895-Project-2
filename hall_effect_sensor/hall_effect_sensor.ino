 //////////// output is staying high even when magnet is not touching, so score increments on it's own//////////
 //////////// tried adding 100nF and 4.7nF but need multimeter to verify output /////////////////////////
 
 // Hall effect sensor 1 signal in pin 46 
 int hall_1 = 46; 
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


void setup() {
  // Configure 14 segment display pins 
  for (int i = 0; i < 8; i++) {
    pinMode(ScoreDisplay[i], OUTPUT);
  }

  // Start serial 
  Serial.begin(115200);

  // Set Pin 46 on ESP32 as input for hall effect sensor 1
  pinMode(hall_1, INPUT); 

}

// Function to increment current score 
void incrementScore() {
  currScore++;
  for (int i = 0; i < 8; i++) {
    bool bit = (currScore >> i) & 1;
    digitalWrite(ScoreDisplay[i], bit ? HIGH : LOW);
  }
}

void loop() {
  int hall_1_state = digitalRead(hall_1); 

  if(hall_1_state == HIGH){
    Serial.println("Magnet is touching"); 
    incrementScore(); 

    getTime = millis(); 

    while(millis() < getTime + 10000){
      // wait 
      Serial.println("waiting"); 
    } 
    Serial.println("done waitng"); 
  }
}
