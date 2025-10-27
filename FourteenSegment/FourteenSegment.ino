// Simply call incrementScore()

const uint8_t a1 = 41;
const uint8_t b1 = 1;
const uint8_t c1 = 2;
const uint8_t d1 = 42;

const uint8_t a2 = 37;
const uint8_t b2 = 40;
const uint8_t c2 = 39;
const uint8_t d2 = 38;

const uint8_t ScoreDisplay[8] = {a2, b2, c2, d2, a1, b1, c1, d1};
byte currScore = 0x00;


void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(ScoreDisplay[i], OUTPUT);
  }
}

void incrementScore() {
  currScore++;
  for (int i = 0; i < 8; i++) {
    bool bit = (currScore >> i) & 1;
    digitalWrite(ScoreDisplay[i], bit ? HIGH : LOW);
  }
}

void loop() {
  for (int i = 0; i < 10; i++) {
    incrementScore();
    delay(1000);
  }
}

/*
b1 1 1
c1 2 2
d1 42 42
a1 41 41


b2 40 40
c2 39 39
d2 38 38
a2 37 37


12, 13, 14, 15
*/