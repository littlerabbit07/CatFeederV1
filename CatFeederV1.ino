#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>

RTC_DS1307 rtc;      // Or RTC_DS3231 if that's your module
Servo myServo;

// Custom microsecond values tuned for your SG90 and feeder
const int CLOSED_US = 1445; // ~85º
const int OPEN_US   = 1695; // ~110º
int lastTriggeredMinute = -1; // Start with "no trigger yet"

// Feeding schedule (hour, minute)
const byte feedingTimes[][2] = {
  {5, 0}, {5, 30},
  {6, 0},
  {7, 0}, {7, 30},
  {12, 0},
  {18, 0}, {18, 30},
  {19, 0},
  {22, 0}, {22, 30},
  {23, 0}
};
const byte feedingCount = sizeof(feedingTimes) / sizeof(feedingTimes[0]);

void moveFeeder() {
  myServo.attach(9);

  // Move to open slowly
  for (int us = CLOSED_US; us <= OPEN_US; us += 5) {
    myServo.writeMicroseconds(us);
    delay(10); // Adjust for speed: higher = slower
  }
  delay(500); // Let it dispense

  // Move back to closed slowly
  for (int us = OPEN_US; us >= CLOSED_US; us -= 5) {
    myServo.writeMicroseconds(us);
    delay(10);
  }
  delay(300); // Let it settle

  myServo.detach(); // Important for longevity

  // Print feed message with timestamp
  DateTime now = rtc.now();
  Serial.print("[");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println("] Feeded automatically.");
}

void setup() {
  Serial.begin(9600);

  // RTC INIT
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, setting the time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Only set the time if you want to update it!
  // Uncomment the next line, upload once, then comment it again.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Servo INIT and feed once on boot
  moveFeeder(); 
  Serial.println("Initial feed done.");
}

void loop() {
  // Manual feed via Serial
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'f') {  // f = feed
      moveFeeder();
      Serial.println("Feeded by manual command.");
    }
  }

  DateTime now = rtc.now();
  byte h = now.hour();
  byte m = now.minute();

  // Check feeding schedule
  for (byte i = 0; i < feedingCount; i++) {
    if (h == feedingTimes[i][0] && m == feedingTimes[i][1]) {
      if (m != lastTriggeredMinute) {
        moveFeeder();
        lastTriggeredMinute = m;
      }
      return; // No need to check further
    }
  }

  // Not a feeding time → reset trigger
  lastTriggeredMinute = -1;

  delay(1000);
}
