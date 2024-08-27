#include <BluetoothSerial.h>

// Definições de pinos
constexpr uint8_t INPUT_LEFT = 25;
constexpr uint8_t INPUT_RIGHT = 33;
constexpr uint8_t INPUT_KICK = 32;
constexpr uint8_t HIT_L_BORDER = 34;
constexpr uint8_t HIT_R_BORDER = 35;
constexpr uint8_t INPUT_TK_DMG = 23;

constexpr uint8_t OUTPUT_EN = 13;
constexpr uint8_t OUTPUT_DIR = 27;
constexpr uint8_t OUTPUT_PULSE = 14;
constexpr uint8_t OUTPUT_KICK = 26;
constexpr uint8_t OUTPUT_BUZZER = 12;

constexpr unsigned short STEP_DELAY = 525;
constexpr unsigned short KICK_DELAY = 200;
constexpr unsigned short KICK_COOLDOWN = 250;

constexpr uint8_t LIFE_LEDS[] = {22, 21, 19, 18, 5, 17, 16, 4, 2, 15};

int8_t curLifeLed = 0;

bool isInputLPressed = false, isInputRPressed = false, isInputKickPressed = false;
bool hitLBorder = false, hitRBorder = false, isTakingDamage = false, tookDamage = false;
bool isKicking = false;

SemaphoreHandle_t xMutex;
TaskHandle_t playerKickTask;

char sBTInput, lastSBTDInput = 'N';

BluetoothSerial SerialBT;

void moveMotor(bool direction);
void kick(void);
void playerKickTaskCode(void *param);
void takeDamage(void);
void initLifeLeds(void);
void toggleLeds(void);
void playInitSong(void);
void playDmgSound(void);
void playGameoverSound(void);

unsigned long lastKickTime = 0;

void moveMotor(bool direction) {
  digitalWrite(OUTPUT_DIR, direction);
  digitalWrite(OUTPUT_EN, LOW);
  digitalWrite(OUTPUT_PULSE, HIGH);
  delayMicroseconds(STEP_DELAY);
  digitalWrite(OUTPUT_PULSE, LOW);
  delayMicroseconds(STEP_DELAY);
  digitalWrite(OUTPUT_EN, HIGH);
}

void kick(void) {
  unsigned long deltaTime = millis() - lastKickTime;
  if (!isKicking && deltaTime > KICK_COOLDOWN) {
    digitalWrite(OUTPUT_KICK, HIGH);
    delay(KICK_DELAY);
    digitalWrite(OUTPUT_KICK, LOW);
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

void takeDamage(void) {
  if (!tookDamage) {
    if (curLifeLed < 9) {
      digitalWrite(LIFE_LEDS[curLifeLed], LOW);
      curLifeLed++;
      playDmgSound();
    } else {
      curLifeLed = 0;

      for (int i = 14; i >= 0; i--) {
        toggleLeds();
        delay(150);
      }

      playGameoverSound();
      initLifeLeds();
    }
  }
}

void initLifeLeds(void) {
  for (int8_t i = 9; i >= 0; i--) {
    digitalWrite(LIFE_LEDS[i], HIGH);
    delay(250);
  }
}

void toggleLeds(void) {
  static bool curState = LOW;

  for (uint8_t pin : LIFE_LEDS) {
    digitalWrite(pin, curState);
  }

  if (curState == LOW) curState = HIGH; else curState = LOW;
}

void playInitSong(void) {
  const uint16_t notes[] = {659, 587, 659, 784, 440, 659, 587, 659, 784, 440, 659, 587, 659, 494, 523};
  const uint16_t durations[] = {250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250};

  for (int i = 0; i < sizeof(notes) / sizeof(*notes); i++) {
    tone(OUTPUT_BUZZER, notes[i], durations[i]);
    delay(durations[i] + 75);
  }
}

void playDmgSound(void) {
  const uint16_t notes[] = {784, 659, 523, 392};
  const uint16_t durations[] = {100, 100, 100, 300};

  for (int i = 0; i < sizeof(notes) / sizeof(*notes); i++) {
    tone(OUTPUT_BUZZER, notes[i], durations[i]);
    delay(durations[i] + 50);
  }
}

void playGameoverSound(void) {
  const uint16_t notes[] = {392, 349, 329, 294, 261};
  const uint16_t durations[] = {300, 300, 300, 600, 1000};

  for (int i = 0; i < sizeof(notes) / sizeof(*notes); i++) {
    tone(OUTPUT_BUZZER, notes[i], durations[i]);
    delay(durations[i] + 50);
  }
}

void setup(void) {
  const uint8_t inputPins[] = {INPUT_LEFT, INPUT_RIGHT, INPUT_KICK, HIT_L_BORDER, HIT_R_BORDER, INPUT_TK_DMG};
  const uint8_t outputPins[] = {OUTPUT_EN, OUTPUT_DIR, OUTPUT_PULSE, OUTPUT_KICK, OUTPUT_BUZZER};

  for (uint8_t pin : inputPins) pinMode(pin, INPUT);
  for (uint8_t pin : outputPins) pinMode(pin, OUTPUT);

  for (uint8_t pin : LIFE_LEDS) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  digitalWrite(OUTPUT_KICK, LOW);
  digitalWrite(OUTPUT_EN, HIGH);

  xMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(playerKickTaskCode, "playerKickTask", 10000, NULL, 0, &playerKickTask, 0);

  SerialBT.begin("Shopa Player");
  Serial.begin(9600);

  initLifeLeds();
  playInitSong();
}

void loop(void) {
  hitLBorder = digitalRead(HIT_L_BORDER);
  hitRBorder = digitalRead(HIT_R_BORDER);
  isTakingDamage = digitalRead(INPUT_TK_DMG);

  if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
    if (!SerialBT.connected()) {
      isInputLPressed = digitalRead(INPUT_LEFT);
      isInputRPressed = digitalRead(INPUT_RIGHT);
      isInputKickPressed = digitalRead(INPUT_KICK);
    } else {
      sBTInput = SerialBT.read();
      switch (sBTInput) {
        case 'L': isInputLPressed = true; lastSBTDInput = sBTInput; break;
        case 'R': isInputRPressed = true; lastSBTDInput = sBTInput; break;
        case 'K': isInputKickPressed = true; break;
        case 'N': 
          if (lastSBTDInput == 'L') isInputLPressed = false;
          if (lastSBTDInput == 'R') isInputRPressed = false;
          break;
      }
    }
    xSemaphoreGive(xMutex);
  }

  if (isInputLPressed && !isInputRPressed && !hitLBorder) moveMotor(true);
  if (isInputRPressed && !isInputLPressed && !hitRBorder) moveMotor(false);
  if (isTakingDamage) takeDamage(); else tookDamage = false;
}