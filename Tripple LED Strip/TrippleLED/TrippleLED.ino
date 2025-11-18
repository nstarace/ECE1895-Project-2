// --- APA102 LED Includes & Definitions ---
#include <APA102.h>
#include <Arduino.h>
#include "HardwareSerial.h"
#include "DFRobotDFPlayerMini.h"

//HardwareSerial mySerial(2);
//DFRobotDFPlayerMini myDFPlayer;

const uint8_t dataPin = 47;
const uint8_t clockPin = 48;
APA102<dataPin, clockPin> ledStrip;

// --- LED State ---
const uint16_t allLED = 60; // Total LEDs on the strip
rgb_color master[allLED];
const uint8_t power = 25; // Overall brightness

// --- Game Constants ---
const int HEIGHT = 10; // How many LEDs tall each column is
const int WIDTH = 3;  // How many columns we have
const long MAX_ROUNDS_SITTING = 50; // Max ticks an item can sit at the bottom

// --- 14-Segment Display Pins ---
const uint8_t a1 = 41;
const uint8_t b1 = 1;
const uint8_t c1 = 2;
const uint8_t d1 = 42;
const uint8_t a2 = 37;
const uint8_t b2 = 40;
const uint8_t c2 = 39;
const uint8_t d2 = 38;
const uint8_t ScoreDisplay[8] = {a2, b2, c2, d2, a1, b1, c1, d1};

// --- Column to LED Mapping ---
// The starting LED index for each of our 3 columns
const int column_start_led[WIDTH] = {0, 20, 40};
const int strike_start_led = 50;
const int PIN_KEY_1 = 45; //strike zone 1 Pressure Sensor
const int PIN_KEY_2 = 16; //strike zone 2 Hall Effect Sensor
const int PIN_KEY_3 = 15; //strike zone 3 Button
const int PIN_KEY_4 = 46; //gyroscope (for "Clean the Plate")

// --- Game State ---
bool game_changed;
bool columns[WIDTH][HEIGHT];
int num_rounds_sitting[WIDTH];
int strikes;
byte score = 0x00;
bool game_over;

// --- 3-Hit Combo Logic ---
int hit_streak[WIDTH];
unsigned long last_hit_time[WIDTH];

// --- Channel 2 (Key '3') Hold Logic ---
bool is_channel_2_holding;
unsigned long channel_2_hold_start_time;
const long channel_2_hold_time_ms = 300; // 300ms hold time

// --- "Clean the Plate" Mini-Game ---
bool is_plate_clean_active;
bool plate_clean_step_1_done;
bool plate_clean_step_2_done;
int last_plate_clean_score;
unsigned long plate_clean_start_time;
const long plate_clean_timeout_ms = 8000;

// --- Item_addition ---
unsigned long last_item_fall_time;
unsigned long last_item_added_time;
long time_between_item_falls_ms;
long time_between_item_adds_ms;

// --- Rhythm/Combo Timing Constants (in milliseconds) ---
const long debounce_delay_ms = 50;
const long min_time_between_hits_ms = 100; // Must wait at least this long after a hit
const long max_time_between_hits_ms = 500; // Must hit before this long passes to continue streak

// --- Input Event Detection ---
int last_key_1_state = HIGH;
int last_key_2_state = HIGH;
int last_key_3_state = HIGH;

// --- DFPlayer Requirements ---
int active_column = 0; // 0, 1, or 2 depending on command
long logTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_KEY_1, INPUT);
  pinMode(PIN_KEY_2, INPUT_PULLUP);
  pinMode(PIN_KEY_3, INPUT);
  pinMode(PIN_KEY_4, INPUT);

  // DFPlayer UART Setup
  //mySerial.begin(9600, SERIAL_8N1, 17, 18);
  //myDFPlayer.setTimeOut(500);  // Serial timeout 500ms
  //myDFPlayer.volume(20);        // Volume 20
  //myDFPlayer.EQ(0);            // Normal equalization
 
  // Configure 14 segment display pins
  for (int i = 0; i < 8; i++) {
    pinMode(ScoreDisplay[i], OUTPUT);
  }
  setup_leds();
 
  init_game();
}

void loop() {
  if (game_over) {
    return;
  }

  //if (logTime + 3000 <= millis()) {
    //logTime = millis();
    //active_column = random(0, 3);
    //if (active_column == 0) {
      //myDFPlayer.playFolder(01, 001);
    //}
    //else if (active_column == 1) {
      //myDFPlayer.playFolder(01, 002);
    //}
    //else {myDFPlayer.playFolder(01, 003); }
  //}

  process_input();

  update_game();

  visualize_game_state(); // This now updates the physical LEDs

  if (strikes >= 3) { GameOver(); }
}

// --- LED Helper Functions ---

void setup_leds() {
  uint8_t bright5bit = bright();
  for (int i = 0; i < allLED; i++) {
    master[i].blue = 0;
    master[i].red = 0;
    master[i].green = 0;
  }
  ledStrip.write(master, allLED, bright5bit);
}

void GameOver() {
  uint8_t bright5bit = bright();
  // Game Over Sequence
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < allLED; j++) {
      master[j].green = 0;
      master[j].blue = 0;
      master[j].red = 255;
    }
    ledStrip.write(master, allLED, bright5bit);
    delay(500);
    for (int j = 0; j < allLED; j++) {
      master[j].red = 0;
    }
    ledStrip.write(master, allLED, bright5bit);
    delay(500);
  }
}

uint8_t bright() {
  uint8_t bright5bit = 1;
  while (bright5bit * 255 < power && bright5bit < 31) { bright5bit++; }
  return bright5bit;
}

// --- Game Logic Functions ---

void init_game() {
  unsigned long now = millis();

  for (int c = 0; c < WIDTH; c++) {
    num_rounds_sitting[c] = 0;
    hit_streak[c] = 0;
    last_hit_time[c] = now;
    for (int r = 0; r < HEIGHT; r++) {
      columns[c][r] = false;
    }
  }

  is_channel_2_holding = false; // *** ADDED BACK ***
  is_plate_clean_active = false;
  plate_clean_step_1_done = false;
  plate_clean_step_2_done = false;
  last_plate_clean_score = -1;

  strikes = 0;
  game_over = false;

  time_between_item_falls_ms = 1000;
  time_between_item_adds_ms = 2000;

  last_item_fall_time = now;
  last_item_added_time = now;

  last_key_1_state = HIGH;
  last_key_2_state = HIGH;
  last_key_3_state = HIGH;
}

void update_game() {
  unsigned long now = millis();
  if (is_plate_clean_active) {
    check_plate_clean_timeout();
    return; // Freeze the game
  }

  if (now > last_item_fall_time + time_between_item_falls_ms) {
    for (int c = 0; c < WIDTH; c++) {
      move_column_items_down(c);
    }
    last_item_fall_time = now;
  }

  if (now > last_item_added_time + time_between_item_adds_ms) {
    int c = random(0, WIDTH); // Pick a random column
    bool available_space = add_item_to_column(c);
    if (!available_space) {
      add_strike(); // This was already uncommented
    }
    last_item_added_time = now;
  }
 
  update_channel_2_hold(); // *** ADDED BACK ***
}

void process_input() {

  if (is_plate_clean_active) {
    process_plate_clean_input();
    return;
  }

  int current_key_1_state = digitalRead(PIN_KEY_1); // Read Pressure sensor
  int current_key_2_state = digitalRead(PIN_KEY_2); // Read button? or hall effect?
  int current_key_3_state = digitalRead(PIN_KEY_3); // Read button #2
  Serial.println(current_key_2_state);
  // *** REMOVED: int current_key_3_state = digitalRead(PIN_KEY_3); ***

  // Check for a new press (state changed from HIGH to LOW)
  if (current_key_1_state == LOW && last_key_1_state == HIGH) {
    process_hit_rhythm(0);
  }
  if (current_key_2_state == LOW && last_key_2_state == HIGH) {
    process_hit_rhythm(1);
  }
  if (current_key_3_state == LOW && last_key_3_state == HIGH) {
    process_hit_rhythm(2);
  }
  // *** REMOVED: rhythm logic for key 3 ***

  last_key_1_state = current_key_1_state;
  last_key_2_state = current_key_2_state;
  last_key_3_state = current_key_3_state;
  // *** REMOVED: last_key_3_state = current_key_3_state; ***
 
  // *** ADDED BACK: old key_3 hold logic ***
  /*bool key_3_is_down = (digitalRead(PIN_KEY_3) == HIGH); // Using original HIGH check

  if (key_3_is_down) {
    if (!is_channel_2_holding) {
      is_channel_2_holding = true;
      channel_2_hold_start_time = millis();
    }
  } else {
    if (is_channel_2_holding) {
      is_channel_2_holding = false;
    }
  }*/
}

void process_plate_clean_input() {
  bool key_4_is_down = (digitalRead(PIN_KEY_4) == LOW);
 
  int current_key_1_state = digitalRead(PIN_KEY_1);
  int current_key_2_state = digitalRead(PIN_KEY_2);

  if (key_4_is_down) {
    if (current_key_1_state == LOW && last_key_1_state == HIGH) {
      plate_clean_step_1_done = true;
    }
   
    if (current_key_2_state == LOW && last_key_2_state == HIGH && plate_clean_step_1_done) {
      plate_clean_step_2_done = true;
    }
  } else {
    plate_clean_step_1_done = false;
    plate_clean_step_2_done = false;
  }

  last_key_1_state = current_key_1_state;
  last_key_2_state = current_key_2_state;

  if (plate_clean_step_1_done && plate_clean_step_2_done) {
    execute_plate_clean();
  }
}

void process_hit_rhythm(int channel) {
  unsigned long now = millis();
  long time_since_last_hit = now - last_hit_time[channel];

  if (time_since_last_hit < debounce_delay_ms) {
    return;
  }

  if (time_since_last_hit >= min_time_between_hits_ms && time_since_last_hit <= max_time_between_hits_ms) {
    hit_streak[channel]++;
  } else {
    hit_streak[channel] = 1;
  }

  last_hit_time[channel] = now;

  if (hit_streak[channel] >= 3) {
    hit_streak[channel] = 0;

    if (columns[channel][0] == true) {
      columns[channel][0] = false;
      num_rounds_sitting[channel] = 0;
      incrementScore();
      check_for_plate_clean_trigger();
     
      if (score > 0 && score % 5 == 0) {
        time_between_item_falls_ms = max(200L, (long)(time_between_item_falls_ms * 0.9));
        time_between_item_adds_ms = max(300L, (long)(time_between_item_adds_ms * 0.9));
      }

    } else {
      // Hit with no piece, reset streak but no strike
      hit_streak[channel] = 0;
    }
  }
}

// *** ADDED BACK: process_hit_hold() function ***
void process_hit_hold(int channel) {
  if (columns[channel][0] == true) {
    columns[channel][0] = false;
    num_rounds_sitting[channel] = 0;
    incrementScore();
    check_for_plate_clean_trigger();
   
    if (score > 0 && score % 5 == 0) {
      time_between_item_falls_ms = max(200L, (long)(time_between_item_falls_ms * 0.9));
      time_between_item_adds_ms = max(300L, (long)(time_between_item_adds_ms * 0.9));
    }

  } else {
    //add_strike(); // Kept commented as in original
  }
}

// *** ADDED BACK: update_channel_2_hold() function ***
void update_channel_2_hold() {
  if (is_channel_2_holding) {
    unsigned long now = millis();
    long hold_duration = now - channel_2_hold_start_time;

    if (hold_duration >= channel_2_hold_time_ms) {
      process_hit_hold(1); // Call hold logic for channel 2
      is_channel_2_holding = false;
    }
  }
}

void move_column_items_down(int column) {
  if (columns[column][0] == true) {
    num_rounds_sitting[column]++;
  } else {
    num_rounds_sitting[column] = 0;
  }

  // Efficiently move items down
  for (int r = 0; r < HEIGHT - 1; r++) {
    if (columns[column][r] == false && columns[column][r + 1] == true) {
      columns[column][r] = true;      
      columns[column][r + 1] = false;
    }
  }

  if (num_rounds_sitting[column] > MAX_ROUNDS_SITTING) {
    add_strike(); // This was already uncommented
    columns[column][0] = false;
    num_rounds_sitting[column] = 0;
  }
}

bool add_item_to_column(int column) {
  if (columns[column][HEIGHT - 1] == true) {
    return false; // Column is full
  }
  columns[column][HEIGHT - 1] = true;
  return true;
}

void add_strike() {
  if (strikes < 4) { // Only increment if not already game over
     strikes++;
  }
}

void check_for_plate_clean_trigger() {
  if (score > 0 && score % 10 == 0 && score != last_plate_clean_score) {
   // activate_plate_clean(); // Uncomment this to enable the mini-game
  }
}

void activate_plate_clean() {
  is_plate_clean_active = true;
  plate_clean_step_1_done = false;
  plate_clean_step_2_done = false;
  plate_clean_start_time = millis();
  last_key_1_state = HIGH;
  last_key_2_state = HIGH;
}

void check_plate_clean_timeout() {
  unsigned long now = millis();
  if (now > plate_clean_start_time + plate_clean_timeout_ms) {
    is_plate_clean_active = false;
    last_plate_clean_score = score;
    add_strike(); // This was already uncommented
  }
}

void execute_plate_clean() {
  for (int c = 0; c < WIDTH; c++) {
    for (int r = 0; r < HEIGHT; r++) {
      columns[c][r] = false;
    }
    num_rounds_sitting[c] = 0;
  }

  is_plate_clean_active = false;
  plate_clean_step_1_done = false;
  plate_clean_step_2_done = false;
  last_plate_clean_score = score;
 
  score += 5; // Bonus 5 points!
  incrementScore(); // Update the display
}

void visualize_game_state() {
  uint8_t bright5bit = bright();

  // 1. Clear the entire LED strip buffer
  for (int i = 0; i < allLED; i++) {
    master[i].red = 0;
    master[i].green = 0;
    master[i].blue = 0;
  }

  // 2. Check for "Clean the Plate" special override
  if (is_plate_clean_active) {
    // Flash all LEDs yellow
    // (millis() / 500) % 2 gives a 0 or 1, switching every 500ms
    if ((millis() / 500) % 2) {
      for (int i = 0; i < allLED; i++) {
        master[i].red = 255;
        master[i].green = 255;
      }
    }
    // else, they stay black (from step 1)
   
  } else {
    // 3. Draw the normal game state
    for (int c = 0; c < WIDTH; c++) {

      // --- Column 2 (Green) ---
      if (c == 2) {
        for (int r = 0; r < HEIGHT; r++) {
          int start_index = column_start_led[c];
          int led_index = start_index + r;

          if (columns[c][r] == true) {
            master[led_index].red = 0;
            master[led_index].blue = 0;
            master[led_index].green = 255;
          }
        }
      }
     
      // --- Column 1 (Blue) ---
      if (c == 1) {
        for (int r = 0; r < HEIGHT; r++) {
          int start_index = column_start_led[c];
         
          // *** THIS IS THE FIX ***
          // This remaps the 'r' index to go in the reverse (upward) direction
          // r=0 (hit zone) now maps to LED 29 (start_index + HEIGHT - 1)
          // r=9 (top) now maps to LED 20 (start_index)
          int led_index = start_index + (HEIGHT - 1) - r;

          if (columns[c][r] == true) {
            master[led_index].blue = 255;
            master[led_index].red = 0;
            master[led_index].green = 0;
          }
        }
      }
     
      // --- Column 0 (Red) ---
      if (c == 0) {
        for (int r = 0; r < HEIGHT; r++) {
          int start_index = column_start_led[c];
          int led_index = start_index + r;

          if (columns[c][r] == true) {
            master[led_index].green = 0;
            master[led_index].blue = 0;
            master[led_index].red = 255;
          }
        }
      }
    }
  }

  // --- Draw Strikes ---
  for (int i = 0; i < 3; i++) {
    if (i < strikes) {
      master[strike_start_led + i].red = 255;
      master[strike_start_led + i].blue = 0;
    }
  }

  // 4. Write the final state to the LEDs
  ledStrip.write(master, allLED, bright5bit);
}

void failure_sequence() {
  game_over = true;
  uint8_t bright5bit = bright();
 
  // Flash all LEDs red 5 times
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < allLED; j++) {
      master[j].blue = 0;
      master[j].red = 255;
      master[j].green = 0;
    }
    ledStrip.write(master, allLED, bright5bit);
    delay(500);
   
    for (int j = 0; j < allLED; j++) {
      master[j].red = 0;
    }
    ledStrip.write(master, allLED, bright5bit);
    delay(500);
  }
}

void incrementScore() {
  score++;
  // Update the 14-segment display
  for (int i = 0; i < 8; i++) {
    bool bit = (score >> i) & 1;
    digitalWrite(ScoreDisplay[i], bit ? HIGH : LOW);
  }
}
