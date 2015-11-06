// Includes ////////////////////////////////////////////////////////////////////////////
#include <LiquidCrystal.h>
#include <AccelStepper.h>

// Motor Definitions ///////////////////////////////////////////////////////////////////
AccelStepper stepper(1, 8, 9);

// Structs /////////////////////////////////////////////////////////////////////////////
//typedef struct button{
//  int prevBtnState;
//  int btnState;
//  const int pin;
//};

// CONSTANTS 
LiquidCrystal lcd(12, 11, 5, 4, 3, 6);
const float ROLLER_DIAMETER = 26; // in mm
const int STEPS_PER_ROTATION = 200;
const int POT_PIN = A0;
const int BUTTON_PIN = 2;
const byte NUM_READINGS = 20;
const long DEBOUNCE_DELAY = 400;  // amount of debounce delay (ms)

// Bender Position 
int curLength = 0;
int curTheta = 0;
int curPhi = 0;

// Object Bend Array Definitions
int lengths [0];
int thetaVals [0];
int phiVals [0];

// Boxcar Average
int readings[NUM_READINGS];
int readIndex = 0;
int total = 0;
int average = 0;
int lastAverage = 0;

// Other variables
int feedLength;
const byte EN = 10;
long lastDebounceTime;

// Setup
void setup() {
  Serial.begin(9600); // set up Serial library at 9600 bps
  
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), debounce, RISING);
  pinMode(POT_PIN, INPUT);
  pinMode(EN, OUTPUT);
  
  stepper.setMaxSpeed(1000);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("FEED LENGTH: ");

  feedLength = 100;
}

// Main Loop
void loop() {
  if (stepper.distanceToGo() != 0) {
    stepper.runSpeedToPosition();
  } else {
    digitalWrite(EN, HIGH);
    int potReading = analogRead(POT_PIN);
  
    total -= readings[readIndex];
    readings[readIndex] = potReading;
    total += readings[readIndex];
    readIndex++;
    if(readIndex >= NUM_READINGS) {
      readIndex = 0;
    }
    lastAverage = average;
    average = total/NUM_READINGS;
  
    if (abs(average/12 - feedLength) > 0.5) {
      feedLength = average/12;
      lcd.setCursor(0, 1); // Set the cursor to the next row
      if (feedLength < 10) {
        lcd.print(0);
      }
      lcd.print(feedLength);
      lcd.print("mm");
    }
  }
  Serial.println(stepper.distanceToGo());
  
//  if(done){
//     cut wire
//  }
  
}

/* 
 *  debouncing code (called by interrupt)
 *  toggles whether or not the scan is running
 */
void debounce(){
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // set a new position for the stepper
    stepper.move(mmToSteps(feedLength));
    stepper.setSpeed(300);
    digitalWrite(EN, LOW);
    
    // update last debounce time
    lastDebounceTime = millis();
  }
}

int mmToSteps(int mm) {
  return STEPS_PER_ROTATION*mm/(ROLLER_DIAMETER*PI);
}

