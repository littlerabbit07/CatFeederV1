#include <Servo.h>

Servo servo;

const int SERVO_PIN = 9;
const int LED_PIN = LED_BUILTIN;
const int BUTTON_PIN = 2;

unsigned long lastMoveTime = 0;
const unsigned long interval = 30000; // 30 seconds

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button active LOW
  servo.attach(SERVO_PIN);
  servo.write(90); // Neutral
  delay(500);
  servo.detach();
}

void loop() {
  unsigned long currentTime = millis();

  // Timed dispense
  if (currentTime - lastMoveTime >= interval) {
    lastMoveTime = currentTime;
    dispenseFood();
  }

  // Manual dispense (button press)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      dispenseFood();
      while (digitalRead(BUTTON_PIN) == LOW); // Wait for button release
    }
  }
}

void dispenseFood() {
  digitalWrite(LED_PIN, HIGH);

  servo.attach(SERVO_PIN);
  servo.write(0);     // Dispense
  delay(500);
  servo.write(90);    // Return
  delay(500);
  servo.detach();

  digitalWrite(LED_PIN, LOW);
}