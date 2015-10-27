/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->	http://www.adafruit.com/products/1438
*/


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *secondMotor = AFMS.getStepper(200, 2);
Adafruit_StepperMotor *firstMotor = AFMS.getStepper(2048, 1);

const byte BUTTON_PIN = 2;        // digital button pin
const long DEBOUNCE_DELAY = 400;  // amount of debounce delay (ms)

long lastDebounceTime;            // last debounce time (ms)
byte feeding;

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  secondMotor->setSpeed(255);  // 10 rpm
  firstMotor->setSpeed(255/4);  // 10 rpm

  // attach debouncing interrupt to button pin
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), debounce, RISING);
  feeding = 0;
}

void loop() {
  if (feeding == 1) {
    Serial.println("Feeding = " + feeding);
    feeding = 0;
  }
  secondMotor->step(1, BACKWARD, MICROSTEP); 
  
//  Serial.println("Single coil steps");
////  secondMotor->step(200, FORWARD, SINGLE); 
//  firstMotor->step(1024, FORWARD, SINGLE); 
////  secondMotor->step(200, BACKWARD, SINGLE); 
//  firstMotor->step(1024, BACKWARD, SINGLE); 

//  Serial.println("Double coil steps");
//  secondMotor->step(200, FORWARD, MICROSTEP); 
//  firstMotor->step(1024, FORWARD, DOUBLE); 
//  delay(5000);
//  secondMotor->step(200, BACKWARD, DOUBLE);
//  firstMotor->step(1024, BACKWARD, DOUBLE);
//  delay(5000);
  
//  Serial.println("Interleave coil steps");
////  secondMotor->step(200, FORWARD, INTERLEAVE); 
//  firstMotor->step(1024, FORWARD, INTERLEAVE); 
////  secondMotor->step(200, BACKWARD, INTERLEAVE); 
//  firstMotor->step(1024, BACKWARD, INTERLEAVE); 
//  
//  Serial.println("Microstep steps");
////  secondMotor->step(50, FORWARD, MICROSTEP); 
//  firstMotor->step(128, FORWARD, MICROSTEP); 
////  secondMotor->step(50, BACKWARD, MICROSTEP);
//  firstMotor->step(128, BACKWARD, MICROSTEP);
}

/* 
 *  debouncing code (called by interrupt)
 *  toggles whether or not data is being collected
 */
void debounce(){
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // change the run state
//    if (feeding == 0) {
//      feeding = 1;
//    }
//    else {
//      feeding = 0;
//    }
    Serial.println("Button pressed!");
    // update last debounce time
    lastDebounceTime = millis();
  }
}
