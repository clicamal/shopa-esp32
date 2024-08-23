#include <BluetoothSerial.h>

const uint8_t INPUT_LEFT = 25;
const uint8_t INPUT_RIGHT = 33;
const uint8_t INPUT_KICK = 32;
const uint8_t HIT_L_BORDER = 34;
const uint8_t HIT_R_BORDER = 35;
const uint8_t INPUT_TK_DMG = 33;

const uint8_t OUTPUT_EN = 12;
const uint8_t OUTPUT_DIR = 27;
const uint8_t OUTPUT_PULSE = 14;
const uint8_t OUTPUT_KICK = 26;

const unsigned short STEP_DELAY = 525;
const unsigned short KICK_DELAY = 200;
const unsigned short KICK_COOLDOWN = 250;

const uint8_t LIFE_LEDS[10] = {
  [0] = 22,
  [1] = 21,
  [2] = 19,
  [3] = 18,
  [4] = 5,
  [5] = 17,
  [6] = 16,
  [7] = 4,
  [8] = 2,
  [9] = 15
};

bool isInputLPressed, isInputRPressed, isInputKickPressed, hitLBorder, hitRBorder;

SemaphoreHandle_t xMutex;

TaskHandle_t playerKickTask;

char sBTInput, lastSBTDInput = 'N';

BluetoothSerial SerialBT;

void playerKickTaskCode(void *param);

bool isKicking = false;
unsigned long lastKickTime = 0;

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

  if (!isKicking && deltaTime > KICK_COOLDOWN) {
    digitalWrite(OUTPUT_KICK, HIGH); // Desativa a solenoide para chute.
    delay(KICK_DELAY);
    digitalWrite(OUTPUT_KICK, LOW); // Recolhe a solenoide depois de um chute.

    lastKickTime = millis();
  }
}

void playerKickTaskCode(void *param) {
  while (true) {
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      if (isInputKickPressed) {
        kick();
        isKicking = true;
      } else {
        isKicking = false;
      }

      isInputKickPressed = false;

      xSemaphoreGive(xMutex);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  String dvcBTName = "Shopa Player";

  pinMode(INPUT_LEFT, INPUT);
  pinMode(INPUT_RIGHT, INPUT);
  pinMode(INPUT_KICK, INPUT);
  pinMode(HIT_L_BORDER, INPUT);
  pinMode(HIT_R_BORDER, INPUT);
  
  pinMode(OUTPUT_EN, OUTPUT);
  pinMode(OUTPUT_DIR, OUTPUT);
  pinMode(OUTPUT_PULSE, OUTPUT);
  pinMode(OUTPUT_KICK, OUTPUT);

  for (short i = 9; i >= 0; i--) {
    pinMode(LIFE_LEDS[i], OUTPUT);
    digitalWrite(LIFE_LEDS[i], LOW);
  }

  digitalWrite(OUTPUT_KICK, LOW); // Inicia a solenoide recolhida.
  digitalWrite(OUTPUT_EN, HIGH); // Inicia o motor desativado.

  xMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    playerKickTaskCode,
    "playerKickTask",
    10000,
    NULL,
    0,
    &playerKickTask,
    0
  );

  SerialBT.begin(dvcBTName);

  Serial.begin(9600);
}

void loop()
{
  hitLBorder = digitalRead(HIT_L_BORDER);
  hitRBorder = digitalRead(HIT_R_BORDER);
  
  if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
    if (!SerialBT.connected()) {
      isInputLPressed = digitalRead(INPUT_LEFT);
      isInputRPressed = digitalRead(INPUT_RIGHT);

      isInputKickPressed = digitalRead(INPUT_KICK);
    }

    else {
      sBTInput = SerialBT.read();

      switch (sBTInput) {
        case 'L':
          isInputLPressed = true;
          lastSBTDInput = sBTInput;
          break;

        case 'R':
          isInputRPressed = true;
          lastSBTDInput = sBTInput;
          break;

        case 'K':
          isInputKickPressed = true;
          break;
        case 'N':
          if (lastSBTDInput == 'L') isInputLPressed = false;
          if (lastSBTDInput == 'R') isInputRPressed = false;
          break;
      }
    }

    xSemaphoreGive(xMutex);
  }

  if (isInputLPressed && !isInputRPressed && !hitLBorder) {
    moveLeft();
  }

  if (isInputRPressed && !isInputLPressed && !hitRBorder) {
    moveRight();
  }
}