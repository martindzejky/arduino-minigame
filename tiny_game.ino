#include <Arduino.h>

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
const int maxShootingTime = 20;

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
    shipPos = 2;
  }
  if (shipPos > 2) {
    shipPos = 0;
  }
}

void renderShip() {
  canvas[shipPos] = true;
}

void updateShooting() {
  if (isShooting) {
    shootTime++;

    if (shootTime > maxShootingTime) {
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

// MAIN

void setup() {
  initializePins();
  
  Serial.begin(9600);
}

void loop() {
  clearCanvas();

  readButtonInput();
  updateIndicator();

  updateShipPos();
  updateShooting();

  renderShip();
  renderShooting();

  renderCanvas();
  delay(1); // for analogRead
}
