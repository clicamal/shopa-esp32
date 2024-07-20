#define INPUT_LEFT 25
#define INPUT_RIGHT 33
#define INPUT_KICK 32
#define HIT_L_BORDER 34
#define HIT_R_BORDER 35

#define OUTPUT_EN 12
#define OUTPUT_DIR 27
#define OUTPUT_PULSE 14
#define OUTPUT_KICK 26

#define STEP_DELAY 525

struct Player {
  bool isKicking;
  unsigned long lastKickTime;

  Player() {
    isKicking = false;
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

    if (!isKicking && deltaTime > 250) {
      digitalWrite(OUTPUT_KICK, HIGH); // Desativa a solenoide para chute.
      delay(200);
      digitalWrite(OUTPUT_KICK, LOW); // Recolhe a solenoide depois de um chute.

      lastKickTime = millis();
    }
  }
};

TaskHandle_t playerKickTask;

Player player;

bool isInputLPressed, isInputRPressed, hitLBorder, hitRBorder;

void playerKickTaskCode(void *param) {
  bool isInputKickPressed;

  while (true) {
    isInputKickPressed = digitalRead(INPUT_KICK);

    if (isInputKickPressed) {
      player.kick();
      player.isKicking = true;
    }

    else {
      player.isKicking = false;
    }
  }
}

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

  digitalWrite(OUTPUT_KICK, LOW); // Inicia a solenoide recolhida.
  digitalWrite(OUTPUT_EN, HIGH); // Inicia o motor desativado.

  xTaskCreatePinnedToCore(
    playerKickTaskCode,
    "playerKickTask",
    10000,
    NULL,
    0,
    &playerKickTask,
    0
  );
}

void loop()
{
  isInputLPressed = digitalRead(INPUT_LEFT);
  isInputRPressed = digitalRead(INPUT_RIGHT);

  hitLBorder = digitalRead(HIT_L_BORDER);
  hitRBorder = digitalRead(HIT_R_BORDER);

  if (isInputLPressed && !isInputRPressed && !hitLBorder) player.moveLeft();
  if (isInputRPressed && !isInputLPressed && !hitRBorder) player.moveRight();
}