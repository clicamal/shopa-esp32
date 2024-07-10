// C++ code

#define INPUT_LEFT 25
#define INPUT_RIGHT 33
#define INPUT_KICK 34
#define HIT_R_BORDER 35
#define HIT_L_BORDER 32

#define OUTPUT_EN 12
#define OUTPUT_PULSE 13
#define OUTPUT_DIR 14
#define OUTPUT_KICK 15

#define STEP_DELAY 525

unsigned long lastKickTime = 0, lastMoveTime = 0;

bool isMoving = false;

void goLeft();
void goRight();
void kick();

void setup()
{
  pinMode(INPUT_LEFT, INPUT);
  pinMode(INPUT_RIGHT, INPUT);
  pinMode(INPUT_KICK, INPUT);
  pinMode(HIT_L_BORDER, INPUT);
  
  pinMode(OUTPUT_EN, OUTPUT);
  pinMode(OUTPUT_PULSE, OUTPUT);
  pinMode(OUTPUT_DIR, OUTPUT);
  pinMode(OUTPUT_KICK, OUTPUT);
}

void loop()
{
  bool isInputLPressed = digitalRead(INPUT_LEFT);
  bool isInputRPressed = digitalRead(INPUT_RIGHT);
  
  bool hitLBorder = digitalRead(HIT_L_BORDER);
  bool hitRBorder = digitalRead(HIT_R_BORDER);
  
  bool isInputKickPressed = digitalRead(INPUT_KICK);
  
  digitalWrite(OUTPUT_KICK, LOW); // Recolhe a solenoide depois de um chute.
  digitalWrite(OUTPUT_EN, HIGH); // Desativa o motor
  
  if (isInputLPressed && !isInputRPressed && !hitLBorder) goLeft();
  if (isInputRPressed && !isInputLPressed && !hitRBorder) goRight();
  
  if (isInputKickPressed) kick();
}

void move() {
  digitalWrite(OUTPUT_EN, LOW);
  digitalWrite(OUTPUT_PULSE, HIGH);
  delayMicroseconds(STEP_DELAY);
  digitalWrite(OUTPUT_PULSE, LOW);
  delayMicroseconds(STEP_DELAY);
} 

void goLeft() {
  digitalWrite(OUTPUT_DIR, HIGH);
  move();
}

void goRight() {
  digitalWrite(OUTPUT_DIR, LOW);
  move();
}

void kick() {
/*   long deltaTime = millis() - lastKickTime;
  
  if (deltaTime > 500) { */
  	digitalWrite(OUTPUT_KICK, HIGH);
/*     lastKickTime = millis();
  } */
}