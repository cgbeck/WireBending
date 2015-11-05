// Includes ////////////////////////////////////////////////////////////////////////////
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

// Motor Definitions ///////////////////////////////////////////////////////////////////
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *feedMotorOne = AFMS.getStepper(200, 1);
Adafruit_StepperMotor *feedMotorTwo = AFMS.getStepper(200, 2);
Adafruit_StepperMotor *feedMotorThree = AFMS.getStepper(200, 2);

// Structs /////////////////////////////////////////////////////////////////////////////
typedef struct button{
  int prevBtnState;
  int btnState;
  const int pin;
};

// Global Definitions //////////////////////////////////////////////////////////////////
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int potentiometerPin = A0;
const int upprBtnPin = A1;

// Roller Size Calculations ////////////////////////////////////////////////////////////
byte pi = 3.14159;
byte rollerRadius;
int rollerCirc = 2*pi*rollerRadius;

// Motor Power Definitions /////////////////////////////////////////////////////////////
byte fullpower = 255;
byte m1Power;
byte m2Power;
byte m3Power;
int pVal;

// Bender Position /////////////////////////////////////////////////////////////////////
int curLength = 0;
int curTheta = 0;
int curPhi = 0;

// Object Bend Array Definitions ///////////////////////////////////////////////////////
int lengths [0];
int thetaVals [0];
int phiVals [0];

// Debounce Stuff //////////////////////////////////////////////////////////////////////
long debounceDelay = 50;
long prevDebounceTime = 0;

// Setup ///////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600); // set up Serial library at 9600 bps
  AFMS.begin(); // create with the default frequency 1.6KHz
//  attachInterrupt(digitalPinToInterrupt(buttonPin), chooseLength, RISING);
  
  pinMode(potentiometerPin, INPUT);
  
  feedMotorOne->setSpeed(0);
  feedMotorTwo->setSpeed(0);
  feedMotorThree->setSpeed(0);
}

// Main Loop ///////////////////////////////////////////////////////////////////////////
void loop() {
  feedSpeedUpdate();
  feedMotorOne->setSpeed(m1Power);
  
//  if(/*done*/){
//     //cut wire
//  }
  
}

// Feed Speed Update Function //////////////////////////////////////////////////////////
void feedSpeedUpdate(){
  pVal = analogRead(potentiometerPin);
  m1Power = pVal/4; // convert the value from the potentiometer to some value between the min and max of the motor speed\
  lcdPrint("Feed Power: ",m1Power);
}

void bend(){
  int i = 0;
  while(/*length(int bends < i*/true){
     break;
  }
}

void lcdPrint(const char* string, int value){
  if ((millis() - prevDebounceTime) > debounceDelay){
     lcd.begin(16, 2);
    lcd.clear();
    lcd.print(string);
    lcd.print(value);
  }
}
//

