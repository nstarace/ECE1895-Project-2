// pressure sensor 1 in pin 0
int press_1 = 0; 
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

void setup(){
  // Configure 14 segment display pins 
  for (int i = 0; i < 8; i++) {
    pinMode(ScoreDisplay[i], OUTPUT);
  }

  // Start serial
  Serial.begin(115200); 

  // Set pin 1 on ESP32 for pressure sensor 1
  pinMode(press_1,INPUT); 
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
  int press_1_state = digitalRead(press_1); 

  if(press_1_state == HIGH){
    incrementScore(); 

    getTime = millis(); 

    while(millis() < getTime + 10000){
      // wait 
      Serial.println("waiting"); 
    }

    Serial.println("done waiting"); 
  }
}

