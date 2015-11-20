#include <LiquidCrystal.h>
#include <AccelStepper.h>

// CONSTANTS 
const float FEED_ROLLER_DIAMETER = 26; // in mm
const int FEED_STEPS_ROTATION = 200;
const float BEND_STEP_DEG = 0.9;
const byte FEED_EN = 4;
const byte BEND_EN = 7;

// VARIABLES
LiquidCrystal lcd(12, 11, 5, 4, 3, 6);
AccelStepper feedStepper(1, 2, 3);
AccelStepper bendStepper(1, 5, 6);

// Setup
void setup() {
  Serial.begin(9600); // set up Serial library at 9600 bps
  
  pinMode(FEED_EN, OUTPUT);
  pinMode(BEND_EN, OUTPUT);
  
  feedStepper.setMaxSpeed(1000);
  bendStepper.setMaxSpeed(1000);
//  stepper.setAcceleration(1000);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("FEED LENGTH: ");
}

// Main Loop
void loop() {
  if (Serial.available() > 0) {
    char letter = Serial.read();
    int actionAmount = Serial.parseInt();
    if (letter == 'f') {
      Serial.print("FEEDING ");
      Serial.print(actionAmount);
      Serial.println(" mm");
      feedStepper.move(-mmToStepsFeed(actionAmount));
      feedStepper.setSpeed(300);
      digitalWrite(FEED_EN, LOW);
    } else if (letter == 'b') {
      Serial.print("BENDING ");
      Serial.print(actionAmount);
      Serial.println(" degrees");
      bendStepper.move(degToStepsBend(actionAmount+actionAmount/20));
      bendStepper.setSpeed(300);
      digitalWrite(BEND_EN, LOW);
    } else {
      Serial.println("INVALID INPUT");
    }
  }
  
  if (feedStepper.distanceToGo() != 0) {
    feedStepper.runSpeedToPosition();
  } else {
    digitalWrite(FEED_EN, HIGH);
  }
  if (bendStepper.distanceToGo() != 0) {
    bendStepper.runSpeedToPosition();
  } else {
    digitalWrite(BEND_EN, HIGH);
  }
}

long mmToStepsFeed(int mm){
  return (long) FEED_STEPS_ROTATION*mm/(FEED_ROLLER_DIAMETER*PI);
}

long degToStepsBend(int deg) {
  return (long) (deg/BEND_STEP_DEG + 0.5);
}

