#include <LiquidCrystal.h>
#include <AccelStepper.h>

// STRUCTS
struct actionItem
{
   byte action;
   int actionAmount; 
};
typedef struct actionItem ActionItem;

// CONSTANTS 
const float BEND_STEP_DEG = 0.9;
const byte BEND_EN = 7;
const byte STATE_READY = 0;
const byte STATE_END = 3;

// VARIABLES
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
AccelStepper bendStepper(1, 5, 6);
byte runState;
//long prevFeedDist;
//long prevBendDist;

// Setup
void setup() {
  Serial.begin(9600); // set up Serial library at 9600 bps
  
//  pinMode(FEED_EN, OUTPUT);
  pinMode(BEND_EN, OUTPUT);
  
//  bendStepper.setEnablePin(BEND_EN);
//  bendStepper.setPinsInverted(false,false,true);
  bendStepper.setSpeed(300);
  bendStepper.setMaxSpeed(300);
  bendStepper.setAcceleration(900);
  digitalWrite(BEND_EN, HIGH);
//  bendStepper.disableOutputs();

//  prevFeedDist = 0;
//  prevBendDist = 0;

  runState = STATE_READY;

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("READY");
}

// Main Loop
void loop() {
  if (runState == STATE_READY) {
    if (Serial.available() > 0) {
      int actionAmount = Serial.parseInt();
      if (actionAmount != 0) {
        Serial.println("STARTING");

        runState = STATE_END;
        Serial.print("bending ");
        Serial.print(actionAmount);
        Serial.println("deg");
  
        bendStepper.move(degToStepsBend(actionAmount));    
//        bendStepper.setSpeed(300);
//        bendStepper.enableOutputs();
        digitalWrite(BEND_EN, LOW);
        delay(100);
        
//        Serial.setTimeout(500);
      }
    }
  } else if (runState = STATE_END) {

    // MOVE THE MOTORS
//    prevFeedDist = feedStepper.distanceToGo();
//    prevBendDist = bendStepper.distanceToGo();
    if (bendStepper.distanceToGo() != 0) {
      bendStepper.run();
    } 
    else {
      Serial.println("COMPLETE");
      
      runState = STATE_READY;
//      prevFeedDist = 0;
//      prevBendDist = 0;
//      bendStepper.disableOutputs();
      digitalWrite(BEND_EN, HIGH);
    }
  }
}

long degToStepsBend(int deg) {
  if(deg > 0) {
    return (long) (deg/BEND_STEP_DEG + 0.5);
  } else {
    return (long) (deg/BEND_STEP_DEG - 0.5);
  }
}

/*
 * Debug serial print statements
 */
//void debug() {
//  Serial.print("\t\t\t\tcur: ");
//  Serial.print(currentActionIndex);
//  Serial.print("\twri: ");
//  Serial.print(writeActionIndex);
//  Serial.print("\t\tact: ");
//  Serial.println(queue[currentActionIndex].action);
//}

