const uint8_t INPUT_LEFT = 25;
const uint8_t INPUT_RIGHT = 33;
const uint8_t INPUT_KICK = 32;
const uint8_t HIT_L_BORDER = 34;
const uint8_t HIT_R_BORDER = 35;

const uint8_t OUTPUT_EN = 12;
const uint8_t OUTPUT_DIR = 27;
const uint8_t OUTPUT_PULSE = 14;
const uint8_t OUTPUT_KICK = 26;

const unsigned short STEP_DELAY = 525;
const unsigned short KICK_DELAY = 200;
const unsigned short KICK_COOLDOWN = 250;

const uint8_t PLAYER_NUM = 1;

void println(const char msg[]) {
  if (Serial.available()) Serial.println(msg);
}

struct Player {
  bool isKicking;
  unsigned long lastKickTime;

  Player(): isKicking(false), lastKickTime(0) {}

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
    println("Movendo para a esquerda.");
    move();
  }

  void moveRight() {
    digitalWrite(OUTPUT_DIR, LOW);
    println("Movendo para a direita.");
    move();
  }

  void kick() {
    unsigned long deltaTime = millis() - lastKickTime;

    if (!isKicking && deltaTime > KICK_COOLDOWN) {
      digitalWrite(OUTPUT_KICK, HIGH); // Desativa a solenoide para chute.
      delay(KICK_DELAY);
      digitalWrite(OUTPUT_KICK, LOW); // Recolhe a solenoide depois de um chute.

      lastKickTime = millis();

      println("Chute.");
    }
  }
};

TaskHandle_t playerKickTask;

Player player;

bool isInputLPressed = false, isInputRPressed = false, hitLBorder = false, hitRBorder = false, isInputKickPressed = false;

SemaphoreHandle_t xMutex;

void playerKickTaskCode(void *param) {
  while (true) {
    xSemaphoreTake(xMutex, portMAX_DELAY);

    isInputKickPressed = digitalRead(INPUT_KICK);

    if (isInputKickPressed) {
      player.kick();
      player.isKicking = true;
    } else {
      player.isKicking = false;
    }

    xSemaphoreGive(xMutex);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  String btDvcName;

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

  xMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    playerKickTaskCode,
    "playerKickTask",
    10000,
    NULL,
    1,
    &playerKickTask,
    0
  );

  Serial.begin(9600);
}

void loop() {
  xSemaphoreTake(xMutex, portMAX_DELAY);

  isInputLPressed = digitalRead(INPUT_LEFT);
  isInputRPressed = digitalRead(INPUT_RIGHT);
  hitLBorder = digitalRead(HIT_L_BORDER);
  hitRBorder = digitalRead(HIT_R_BORDER);

  if (isInputLPressed && !isInputRPressed && !hitLBorder) {
    player.moveLeft();
    isInputLPressed = false;
  }

  if (isInputRPressed && !isInputLPressed && !hitRBorder) {
    player.moveRight();
    isInputRPressed = false;
  }

  xSemaphoreGive(xMutex);
}