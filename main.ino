#define INPUT_LEFT 25
#define INPUT_RIGHT 33
#define INPUT_KICK 32
#define HIT_L_BORDER 34
#define HIT_R_BORDER 35

#define OUTPUT_EN 12
#define OUTPUT_DIR 13
#define OUTPUT_PULSE 14
#define OUTPUT_KICK 26

#define STEP_DELAY 525

struct Player {
  unsigned long lastKickTime;

  Player() {
    lastKickTime = 0;
  }

  void move() {
    digitalWrite(OUTPUT_EN, LOW); // Ativa o motor.
    digitalWrite(OUTPUT_PULSE, HIGH); // Dá um pulso no motor.
    delayMicroseconds(STEP_DELAY);
    digitalWrite(OUTPUT_PULSE, LOW);
    delayMicroseconds(STEP_DELAY);
    digitalWrite(OUTPUT_EN, HIGH); // Desativa o motor.
  }

  void moveLeft() {
    digitalWrite(OUTPUT_DIR, HIGH);
    move();
  }

  void moveRight() {
    digitalWrite(OUTPUT_DIR, LOW);
    move();
  }

  void kick() {
    unsigned long deltaTime = millis() - lastKickTime;
  
    if (deltaTime > 650) {
      digitalWrite(OUTPUT_KICK, LOW); // Desativa a solenoide para chute.
      delay(250);
      digitalWrite(OUTPUT_KICK, HIGH); // Recolhe a solenoide depois de um chute.
      
      lastKickTime = millis();
    }
  }

  void update() {
    bool isInputLPressed = digitalRead(INPUT_LEFT);
    bool isInputRPressed = digitalRead(INPUT_RIGHT);
    
    bool hitLBorder = digitalRead(HIT_L_BORDER);
    bool hitRBorder = digitalRead(HIT_R_BORDER);
    
    bool isInputKickPressed = digitalRead(INPUT_KICK);
    
    if (isInputLPressed && !isInputRPressed && !hitLBorder) moveLeft();
    if (isInputRPressed && !isInputLPressed && !hitRBorder) moveRight();
    
    if (isInputKickPressed) kick();
  }
};

Player player;

void setup()
{
  pinMode(INPUT_LEFT, INPUT);
  pinMode(INPUT_RIGHT, INPUT);
  pinMode(INPUT_KICK, INPUT);
  pinMode(HIT_L_BORDER, INPUT);
  pinMode(HIT_R_BORDER, INPUT);
  
  pinMode(OUTPUT_EN, OUTPUT);
  pinMode(OUTPUT_DIR, OUTPUT);
  pinMode(OUTPUT_PULSE, OUTPUT);
  pinMode(OUTPUT_KICK, OUTPUT);

  digitalWrite(OUTPUT_KICK, HIGH); // Inicia a solenoide recolhida.
  digitalWrite(OUTPUT_EN, HIGH); // Inicia o motor desativado.
}

void loop()
{
  player.update();
}