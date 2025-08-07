#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>

RTC_DS1307 rtc;      // Or RTC_DS3231 if that's your module
Servo myServo;

// Custom microsecond values tuned for your SG90 and feeder
const int CLOSED_US = 1445; // ~85º
const int OPEN_US   = 1695; // ~110º
int lastTriggeredMinute = -1; // Start with "no trigger yet"


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


  // Servo INIT and move and feed
  moveFeeder(); // RESET Feed
  Serial.println("] Feeded by button.");

}

void loop() {

  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'f') {  // f = feed
      moveFeeder(); // Test feed
      Serial.println("] Feeded by web ");  
    }
  }

  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();

  // Check if current time matches any schedule
  if ((h == 6 && (m == 0 || m == 30)) ||
      (h == 7 && m == 0) ||
      (h == 12 && (m == 0 || m == 30)) ||
      (h == 13 && m == 0) ||
      (h == 17 && m == 0) ||
      (h == 18 && m == 0) ||
      (h == 19 && m == 0) ||
      (h == 20 && m == 0) ||
      (h == 21 && m == 0)) {

    if (m != lastTriggeredMinute) {
      moveFeeder(); // Test feed
      lastTriggeredMinute = m;
    }

  } else {
    lastTriggeredMinute = -1;  // Reset trigger control
  }

  delay(1000);
}
