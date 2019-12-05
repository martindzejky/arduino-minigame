// TYPES

struct Enemy {
  bool alive = false;
  short x;
  short y;
  short timer = 0;
};

// CANVAS

// canvas data used to "buffer" rendering
bool canvas[9] = {
  false, false, false,
  false, false, false,
  false, false, false
};

// lookup table for canvas -> pin
int canvasPins[9] = {
  12, 10, 11,
  9, 7, 8,
  6, 4, 5
};

// STATE

// input

bool leftPressed = false;
bool rightPressed = false;
bool shootPressed = false;

bool wasLeftPressed = false;
bool wasRightPressed = false;
bool wasShootPressed = false;

// "ship"

short shipPos = 1;

bool isShooting = false;
int shootTime = 0;
const int MAX_SHOOTING_TIME = 20;

// enemies

const int MAX_ENEMIES = 6;
const float ENEMY_MOVE_TIMER_MIN = 200.f;
const float ENEMY_MOVE_TIMER_MAX = 900.f;
const float ENEMY_SPAWN_MIN = 800.f;
const float ENEMY_SPAWN_MAX = 10000.f;
const float ENEMY_SPAWN_THRESHOLD = 10.f;
const float GAME_TIME_MAX = 60000.f;

Enemy enemies[MAX_ENEMIES];

// game

bool isEnd = false;
bool endFlash = false;

// HELPERS

void initializePins() {
  pinMode(LED_BUILTIN, OUTPUT);

  // LED display pins
  for (int i = 4; i <= 12; i++) {
    pinMode(i, OUTPUT);
  }
}

void readButtonInput() {
  wasLeftPressed = leftPressed;
  wasRightPressed = rightPressed;
  wasShootPressed = shootPressed;

  leftPressed = analogRead(5) > 1000;
  rightPressed = analogRead(4) > 1000;
  shootPressed = analogRead(2) > 1000;
}

void printButtonState() {
  Serial.print("Left: ");
  Serial.print(leftPressed);

  Serial.print("; Right: ");
  Serial.print(rightPressed);

  Serial.print("; Shoot: ");
  Serial.print(shootPressed);

  Serial.println();
}

void updateIndicator() {
  if (leftPressed || rightPressed || shootPressed) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void updateShipPos() {
  if (leftPressed && !wasLeftPressed) {
    shipPos--;
  }
  if (rightPressed && !wasRightPressed) {
    shipPos++;
  }

  if (shipPos < 0) {
    shipPos = 0;
  }
  if (shipPos > 2) {
    shipPos = 2;
  }
}

void renderShip() {
  canvas[shipPos] = true;
}

void updateShooting() {
  if (isShooting) {
    shootTime++;

    if (shootTime > MAX_SHOOTING_TIME) {
      isShooting = false;
    }
  } else {
    if (shootPressed && !wasShootPressed) {
      isShooting = true;
      shootTime = 0;
    }
  }
}

void renderShooting() {
  if (isShooting) {
    if (shipPos == 0) {
      canvas[3] = true;
      canvas[6] = true;
    } else if (shipPos == 1) {
      canvas[4] = true;
      canvas[7] = true;
    } else  {
      canvas[5] = true;
      canvas[8] = true;
    } 
  }
}

void clearCanvas() {
  for (int i = 0; i < 9; i++) {
    canvas[i] = false;
  }
}

void renderCanvas() {
  for (int i = 0; i < 9; i++) {
    int pin = canvasPins[i];
    digitalWrite(pin, canvas[i] ? HIGH : LOW);
  }
}

void createEnemies() {
  enemies[0].alive = true;
  enemies[0].x = 0;
  enemies[0].y = 1;

  enemies[1].alive = true;
  enemies[1].x = 2;
  enemies[1].y = 2;
}

void spawnEnemies() {
  float chance = ENEMY_SPAWN_MAX -
    (
      (millis() / GAME_TIME_MAX) *
      (ENEMY_SPAWN_MAX - ENEMY_SPAWN_MIN)
    );

  if (chance < ENEMY_SPAWN_MIN) {
    chance = ENEMY_SPAWN_MIN;
  }

  if (random(chance) < ENEMY_SPAWN_THRESHOLD) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].alive) continue;

      enemies[i].x = random(3);
      enemies[i].y = 2;
      enemies[i].alive = true;
      enemies[i].timer = 0;

      break;
    }
  }
}

void updateEnemies() {
  float maxTimer = ENEMY_MOVE_TIMER_MAX -
    (
      (millis() / GAME_TIME_MAX) *
      (ENEMY_MOVE_TIMER_MAX - ENEMY_MOVE_TIMER_MIN)
    );

  if (maxTimer < ENEMY_MOVE_TIMER_MIN) {
    maxTimer = ENEMY_MOVE_TIMER_MIN;
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) continue;

    if (isShooting && shipPos == enemies[i].x) {
      enemies[i].alive = false;
      continue;
    }

    enemies[i].timer++;

    if (enemies[i].timer > maxTimer) {
      if (enemies[i].y > 0) {
        enemies[i].y--;
        enemies[i].timer = 0;
      } else {
        enemies[i].alive = false;
        isEnd = true;
      }
    }
  }
}

void renderEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].alive) {
      canvas[
          enemies[i].y * 3
        + enemies[i].x
      ] = true;
    }
  }
}

void renderEnd() {
  for (int i = 0; i < 9; i++) {
    canvas[i] = endFlash;
  }

  endFlash = !endFlash;
}

// MAIN

void setup() {
  initializePins();
  createEnemies();
  
  Serial.begin(9600);
  randomSeed(analogRead(0));
}

void loop() {
  if (isEnd) {
    renderEnd();
    renderCanvas();
    delay(1000);
    return;
  }

  clearCanvas();

  readButtonInput();
  updateIndicator();
  spawnEnemies();

  updateShipPos();
  updateShooting();
  updateEnemies();

  renderShip();
  renderShooting();
  renderEnemies();

  renderCanvas();
  delay(1); // for analogRead
}
