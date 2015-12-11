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
int numReceived;

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
  numReceived = 0;

  runState = STATE_READY;

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("READY");
}

// Main Loop
void loop() {
  if (runState == STATE_READY) {
    if (Serial.available() > 0) {
      String input = Serial.readString();
      if (input = "START") {
        // REINITIALIZE VARIABLES SOMEWHERE
//        currentActionIndex = 2;
//        writeActionIndex = 0;
        runState = STATE_INIT;
        Serial.println("SEND_COMMAND STATE_READY");
        Serial.print("\t\t\t\tcur: ");
        Serial.print(currentActionIndex);
        Serial.print("\twri: ");
        Serial.print(writeActionIndex);
        Serial.print("\t\tact: ");
        Serial.println(queue[currentActionIndex].action);
      }
    }
  } else if (runState == STATE_INIT) {
    if(receiveCommand()) {
      if(performAction()) {
        runState = STATE_RUNNING;
      } else {
        // something went wrong, reset to ready state
        runState == STATE_READY;
      }
    }
  }
  else if (runState == STATE_RUNNING) {
//    if (Serial.available() > 0) {
//      char letter = Serial.read();
//      int actionAmount = Serial.parseInt();
//  
//      if(letter == 'F' || letter == 'B') {
//        if(letter == 'F') {
//          queue[writeActionIndex].action = ACTION_FEED;
//        } else if (letter == 'B') {
//          queue[writeActionIndex].action = ACTION_BEND;
//        }
//        queue[writeActionIndex].actionAmount = actionAmount;
//        writeActionIndex = incrementQueueIndex(writeActionIndex);
//        
//        if (queue[writeActionIndex].action == ACTION_NONE) {
//          Serial.println("SEND_COMMAND");
//        } 
////        Serial.print("currentActionIndex: ");
////        Serial.println(currentActionIndex);
//      }
//    }

    receiveCommand();

    // move the motors
    prevFeedDist = feedStepper.distanceToGo();
    prevBendDist = bendStepper.distanceToGo();
    if (prevFeedDist != 0) {
      feedStepper.run();
    }
    if (prevBendDist != 0) {
      bendStepper.run();
    }

    // perform another action in the queue if current action is finished
    if (feedStepper.distanceToGo() == 0 && bendStepper.distanceToGo() == 0) {
      if (prevFeedDist > 0 || prevBendDist > 0) {
        if (numReceived >= QUEUE_LENGTH) {
          Serial.println("SEND_COMMAND STATE_RUNNING");
        }
        queue[currentActionIndex].action = ACTION_NONE;
        queue[currentActionIndex].actionAmount = 0;
        currentActionIndex = incrementQueueIndex(currentActionIndex);
        feedStepper.disableOutputs();
        bendStepper.disableOutputs();
        Serial.println("finished action");
        Serial.print("\t\t\t\tcur: ");
        Serial.print(currentActionIndex);
        Serial.print("\twri: ");
        Serial.print(writeActionIndex);
        Serial.print("\t\tact: ");
        Serial.println(queue[currentActionIndex].action);
      }
      
//      if (queue[currentActionIndex].action != ACTION_NONE) {   
//        if (queue[currentActionIndex].action == ACTION_FEED) {
//          Serial.print("feeding ");
//          Serial.print(queue[currentActionIndex].actionAmount);
//          Serial.println(" mm");
//  
//          feedStepper.move(-mmToStepsFeed(queue[currentActionIndex].actionAmount));
//          feedStepper.setSpeed(300);
//          feedStepper.enableOutputs();
//          delay(100);
//        } else if (queue[currentActionIndex].action == ACTION_BEND) {
//          Serial.print("bending ");
//          Serial.print(queue[currentActionIndex].actionAmount);
//          Serial.println(" degrees");
//  
//          bendStepper.move(degToStepsBend(queue[currentActionIndex].actionAmount));
//          bendStepper.setSpeed(300);
//          bendStepper.enableOutputs();
//          delay(100);
//        }
//      }
      performAction();
    }
  } else if (runState = STATE_END) {
    // move the motors
    prevFeedDist = feedStepper.distanceToGo();
    prevBendDist = bendStepper.distanceToGo();
    if (prevFeedDist != 0) {
      feedStepper.run();
    }
    if (prevBendDist != 0) {
      bendStepper.run();
    }

    // perform another action in the queue if current action is finished
    if (feedStepper.distanceToGo() == 0 && bendStepper.distanceToGo() == 0) {
      if (prevFeedDist > 0 || prevBendDist > 0) {
        queue[currentActionIndex].action = ACTION_NONE;
        queue[currentActionIndex].actionAmount = 0;
        currentActionIndex = incrementQueueIndex(currentActionIndex);
        feedStepper.disableOutputs();
        bendStepper.disableOutputs();
        Serial.println("finished action");
        Serial.print("\t\t\t\tcur: ");
        Serial.print(currentActionIndex);
        Serial.print("\twri: ");
        Serial.print(writeActionIndex);
        Serial.print("\t\tact: ");
        Serial.println(queue[currentActionIndex].action);
      }
      
      performAction();

      if (queue[currentActionIndex].action == ACTION_NONE) {
        Serial.println("COMPLETE");
        Serial.print("\t\t\t\tcur: ");
        Serial.print(currentActionIndex);
        Serial.print("\twri: ");
        Serial.print(writeActionIndex);
        Serial.print("\t\tact: ");
        Serial.println(queue[currentActionIndex].action);
        runState = STATE_READY;
        currentActionIndex = 0;
        writeActionIndex = 0;
        numReceived = 0;
        prevFeedDist = 0;
        prevBendDist = 0;
        feedStepper.disableOutputs();
        bendStepper.disableOutputs();
      }
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
        numReceived++;
        Serial.print("received FEED");
        Serial.println(actionAmount);
        if (numReceived < QUEUE_LENGTH) {
          Serial.println("SEND_COMMAND receieveCommand");
          Serial.print("\t\t\t\tcur: ");
          Serial.print(currentActionIndex);
          Serial.print("\twri: ");
          Serial.print(writeActionIndex);
          Serial.print("\t\tact: ");
          Serial.println(queue[currentActionIndex].action);
        }
        return true;
      }
      case 'B': {
        int actionAmount = Serial.parseInt();
        queue[writeActionIndex].action = ACTION_BEND;
        queue[writeActionIndex].actionAmount = actionAmount;
        writeActionIndex = incrementQueueIndex(writeActionIndex);
        numReceived++;
        Serial.print("received BEND");
        Serial.println(actionAmount);
        if (numReceived < QUEUE_LENGTH) {
          Serial.println("SEND_COMMAND receiveCommand");
          Serial.print("\t\t\t\tcur: ");
          Serial.print(currentActionIndex);
          Serial.print("\twri: ");
          Serial.print(writeActionIndex);
          Serial.print("\t\tact: ");
          Serial.println(queue[currentActionIndex].action);
        }
        return true;
      }
      case 'E': {
        String input = Serial.readString();
        if (input == "ND") {
          runState = STATE_END;
        }
        return true;
      }
      default: return false;
    }
  }
  return false;
}

bool performAction() {
  if (queue[currentActionIndex].action != ACTION_NONE) {   
    if (queue[currentActionIndex].action == ACTION_FEED) {
      Serial.print("feeding ");
      Serial.print(queue[currentActionIndex].actionAmount);
      Serial.println(" mm");

      feedStepper.move(-mmToStepsFeed(queue[currentActionIndex].actionAmount));
      feedStepper.setSpeed(300);
      feedStepper.enableOutputs();
//      delay(100); // TODO: FIX THE DELAYS TO BE NON-BLOCKING
      return true;
    } else if (queue[currentActionIndex].action == ACTION_BEND) {
      Serial.print("bending ");
      Serial.print(queue[currentActionIndex].actionAmount);
      Serial.println(" degrees");

      bendStepper.move(degToStepsBend(queue[currentActionIndex].actionAmount));
      bendStepper.setSpeed(300);
      bendStepper.enableOutputs();
//      delay(100);
      return true;
    }
  }
  return false;
}

