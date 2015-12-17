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
const float FEED_ROLLER_DIAMETER = 19.5; // in mm
const int FEED_STEPS_ROTATION = 200;
const float BEND_STEP_DEG = 0.9;
const byte FEED_EN = 4;
const byte BEND_EN = 7;
const byte ACTION_NONE = 0;
const byte ACTION_FEED = 1;
const byte ACTION_BEND = 2;
const byte ACTION_FINISHED = 3;
const byte STATE_READY = 0;
const byte STATE_INIT = 1;
const byte STATE_RUNNING = 2;
const byte STATE_END = 3;
const byte QUEUE_LENGTH = 3;

// VARIABLES
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
AccelStepper feedStepper(1, 2, 3);
AccelStepper bendStepper(1, 5, 6);
ActionItem queue[QUEUE_LENGTH];
byte currentActionIndex;
byte writeActionIndex;
byte runState;
long prevFeedDist;
long prevBendDist;
unsigned long completeTime;
byte completeFlash;

// Setup
void setup() {
  Serial.begin(9600); // set up Serial library at 9600 bps
  
//  pinMode(FEED_EN, OUTPUT);
//  pinMode(BEND_EN, OUTPUT);

  feedStepper.setEnablePin(FEED_EN);
  feedStepper.setPinsInverted(false,false,true);
  feedStepper.setSpeed(400);
  feedStepper.setMaxSpeed(400);
  feedStepper.setAcceleration(2000);
  feedStepper.disableOutputs();
  
  bendStepper.setEnablePin(BEND_EN);
  bendStepper.setPinsInverted(false,false,true);
  bendStepper.setSpeed(300);
  bendStepper.setMaxSpeed(300);
  bendStepper.setAcceleration(900);
  bendStepper.disableOutputs();

  currentActionIndex = 0;
  writeActionIndex = 0;
  for (int i = 0; i < sizeof(queue)/sizeof(ActionItem); i++) {
    queue[i].action = ACTION_NONE;
    queue[i].actionAmount = 0;
  }

  prevFeedDist = 0;
  prevBendDist = 0;

  runState = STATE_READY;
  completeFlash = 0;
  completeTime = millis() - 6000;

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("READY");
}

// Main Loop
void loop() {
  if (runState == STATE_READY) {  // wait for START messaage
    if (millis() - completeTime < 5995) {
      if ((millis() - completeTime)%1090 < 545 && completeFlash == 0) {
        lcd.print("COMPLETE");
        completeFlash = 1;
      } else if ((millis() - completeTime)%1090 > 545 && completeFlash == 1) {
        lcd.clear();
        completeFlash = 0;
      }
    } else {
      if (completeFlash == 1) {
        lcd.clear();
        lcd.print("READY");
        completeFlash = 0;
      }
    }
    
    if (Serial.available() > 0) {
      String input = Serial.readString();
      if (input == "START") {
        Serial.println(input);
        lcd.clear();
        lcd.print("INITIALIZING");
        runState = STATE_INIT;
        Serial.setTimeout(200);
        Serial.println("SEND_COMMAND");
      }
    }
  } else if (runState == STATE_INIT) {  // fill the queue first
    if (receiveCommand()) {
      if (runState == STATE_INIT) {
        if (writeActionIndex != 0) {  // queue is not filled yet
          Serial.println("SEND_COMMAND");
        } else {  // queue is full
          Serial.println("queue full");
          initAction();
          runState = STATE_RUNNING;
        }
      } else if (runState == STATE_END) {
        initAction();
      }
    }
  }
  else if (runState == STATE_RUNNING) {   // perform actions in queue, 
    // receive a command TODO: CREATE FLAG FOR RECEIVE
    receiveCommand(); //***********************************

    // move the motors
    moveMotors();    

    // perform another action in the queue if current action is finished
    checkActionCompleted();
  } else if (runState = STATE_END) {
    
    // move the motors
    moveMotors();

    // perform another action in the queue if current action is finished
    checkActionCompleted();

    // complete!
    if (queue[currentActionIndex].action == ACTION_NONE) {
      Serial.println("COMPLETE");
      Serial.setTimeout(1000);
      lcd.clear();
      completeTime = millis();
      runState = STATE_READY;
      currentActionIndex = 0;
      writeActionIndex = 0;
      prevFeedDist = 0;
      prevBendDist = 0;
      feedStepper.disableOutputs();
      bendStepper.disableOutputs();
    }
  }
}

long mmToStepsFeed(int mm){
  return (long) FEED_STEPS_ROTATION*mm/(FEED_ROLLER_DIAMETER*PI);
}

long degToStepsBend(int deg) {
  if(deg > 0) {
    return (long) (deg/BEND_STEP_DEG + 0.5);
  } else {
    return (long) (deg/BEND_STEP_DEG - 0.5);
  }
}

byte incrementQueueIndex(byte index) {
  return ++index%QUEUE_LENGTH;
}

/*
 * returns: true if a command was received
 */
bool receiveCommand() {
  if (Serial.available() > 0) {
    char letter = Serial.read();
    switch (letter) {
      case 'F': {
        int actionAmount = Serial.parseInt();
        queue[writeActionIndex].action = ACTION_FEED;
        queue[writeActionIndex].actionAmount = actionAmount;
        writeActionIndex = incrementQueueIndex(writeActionIndex);
        Serial.print("received FEED");
        Serial.println(actionAmount);

        return true;
      }
      case 'B': {
        int actionAmount = Serial.parseInt();
        queue[writeActionIndex].action = ACTION_BEND;
        queue[writeActionIndex].actionAmount = actionAmount;
        writeActionIndex = incrementQueueIndex(writeActionIndex);
        Serial.print("received BEND");
        Serial.println(actionAmount);

        return true;
      }
      case 'E': {
        String input = Serial.readString();
        if (input == "ND") {
          Serial.println("received END");
          runState = STATE_END;
        }
        return true;
      }
      default: return false;
    }
  }
  return false;
}

/*
 * returns: true if a motor is moved
 */
bool moveMotors() {
  bool moved = false;
  prevFeedDist = feedStepper.distanceToGo();
  prevBendDist = bendStepper.distanceToGo();
  if (prevFeedDist != 0) {
    feedStepper.run();
    moved = true;
  }
  if (prevBendDist != 0) {
    bendStepper.run();
    moved = true;
  }
  return moved;
}

/*
 * returns: true if action completed
 */
bool checkActionCompleted() {
  if (feedStepper.distanceToGo() == 0 && bendStepper.distanceToGo() == 0) {
//    Serial.println("\t\t\t\t\t\tboth distances to go are zero");
    if (prevFeedDist != 0 || prevBendDist != 0) {
      if (runState != STATE_END) {
        Serial.println("SEND_COMMAND");
      }
      Serial.print("finished ");
      if (queue[currentActionIndex].action == ACTION_FEED) {
        Serial.print("feeding ");
        Serial.print(queue[currentActionIndex].actionAmount);
        Serial.println("mm");
      } else if (queue[currentActionIndex].action == ACTION_BEND) {
        Serial.print("bending ");
        Serial.print(queue[currentActionIndex].actionAmount);
        Serial.println("deg");
      }
      
      queue[currentActionIndex].action = ACTION_NONE;
      queue[currentActionIndex].actionAmount = 0;
      currentActionIndex = incrementQueueIndex(currentActionIndex);
      feedStepper.disableOutputs();
      bendStepper.disableOutputs();

      initAction();
      return true;
    }
  }
  return false;
}

/*
 * returns: true if nnew action is initiated
 */
bool initAction() {
  if (queue[currentActionIndex].action != ACTION_NONE) {   
    if (queue[currentActionIndex].action == ACTION_FEED) {
      Serial.print("feeding ");
      Serial.print(queue[currentActionIndex].actionAmount);
      Serial.println("mm");

      lcd.clear();
      lcd.print("FEEDING ");
      lcd.print(queue[currentActionIndex].actionAmount);
      lcd.print("mm");
      lcd.setCursor(0, 1);
      lcd.print((char)0x7e);
      ActionItem nextItem = queue[incrementQueueIndex(currentActionIndex)];
      if (nextItem.action == ACTION_BEND) {
        lcd.print("BEND ");
        lcd.print(nextItem.actionAmount);
        lcd.print((char)0xdf);
      } else if (nextItem.action == ACTION_FEED) {
        lcd.print("FEED ");
        lcd.print(nextItem.actionAmount);
        lcd.print("mm");
      } else if (nextItem.action == ACTION_NONE && runState == STATE_RUNNING) {
        lcd.print("WAIT FOR NEXT");
        lcd.print(nextItem.actionAmount);
        lcd.print("mm");
      } else if (nextItem.action == ACTION_NONE && runState == STATE_END) {
        lcd.print("END");
      }

      feedStepper.move(-mmToStepsFeed(queue[currentActionIndex].actionAmount));
      feedStepper.enableOutputs();
      delay(100);
      return true;
    } else if (queue[currentActionIndex].action == ACTION_BEND) {
      Serial.print("bending ");
      Serial.print(queue[currentActionIndex].actionAmount);
      Serial.println("deg");

      lcd.clear();
      lcd.print("BENDING ");
      lcd.print(queue[currentActionIndex].actionAmount);
      lcd.print((char)0xdf);
      lcd.setCursor(0, 1);
      lcd.print((char)0x7e);
      ActionItem nextItem = queue[incrementQueueIndex(currentActionIndex)];
      if (nextItem.action == ACTION_BEND) {
        lcd.print("BEND ");
        lcd.print(nextItem.actionAmount);
        lcd.print((char)0xdf);
      } else if (nextItem.action == ACTION_FEED) {
        lcd.print("FEED ");
        lcd.print(nextItem.actionAmount);
        lcd.print("mm");
      } else if (nextItem.action == ACTION_NONE && runState == STATE_RUNNING) {
        lcd.print("WAIT FOR NEXT");
        lcd.print(nextItem.actionAmount);
        lcd.print("mm");
      } else if (nextItem.action == ACTION_NONE && runState == STATE_END) {
        lcd.print("END");
      }

      bendStepper.move(degToStepsBend(queue[currentActionIndex].actionAmount));
      bendStepper.enableOutputs();
      delay(100);
      return true;
    }
  }
  return false;
}

/*
 * Debug serial print statements
 */
void debug() {
  Serial.print("\t\t\t\tcur: ");
  Serial.print(currentActionIndex);
  Serial.print("\twri: ");
  Serial.print(writeActionIndex);
  Serial.print("\t\tact: ");
  Serial.println(queue[currentActionIndex].action);
}

