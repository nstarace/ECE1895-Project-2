 int hall_1 = 46; 


void setup() {
  // Set Pin 46 on ESP32 as input for hall effect sensor 1
  pinMode(hall_1, INPUT); 
  Serial.begin(115200); 
}

void loop() {
  int hall_1_state = digitalRead(hall_1); 

  if(hall_1_state == HIGH){
    Serial.println("Magnet is touching"); 
  }
  else{
    Serial.println("Magnet is not touching"); 
  }
}
