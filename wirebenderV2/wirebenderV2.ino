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
  if (runState == STATE_READY) {  // wait for START messaage
    if (Serial.available() > 0) {
      String input = Serial.readString();
      if (input = "START") {
        // REINITIALIZE VARIABLES SOMEWHERE
        runState = STATE_INIT;
        Serial.setTimeout(500);
        Serial.println("SEND_COMMAND STATE_INIT");
      }
    }
  } else if (runState == STATE_INIT) {  // fill the queue first
    if (receiveCommand()) {
      if (runState == STATE_INIT) {
        if (writeActionIndex != 0) {  // queue is not filled yet
          Serial.println("SEND_COMMAND STATE_INIT");
        } else {  // queue is full
          Serial.println("queue full");
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
//    Serial.println(prevBendDist);
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

      feedStepper.move(-mmToStepsFeed(queue[currentActionIndex].actionAmount));
      feedStepper.setSpeed(300);
      feedStepper.enableOutputs();
      delay(100); // TODO: FIX THE DELAYS TO BE NON-BLOCKING
//      receiveCommand();
      return true;
    } else if (queue[currentActionIndex].action == ACTION_BEND) {
      Serial.print("bending ");
      Serial.print(queue[currentActionIndex].actionAmount);
      Serial.println("deg");

      bendStepper.move(degToStepsBend(queue[currentActionIndex].actionAmount));
      Serial.println(degToStepsBend(queue[currentActionIndex].actionAmount));
      // TODO:FIX BROKEN NEGATIVE BENDING


      
      bendStepper.setSpeed(300);
      bendStepper.enableOutputs();
      delay(100);
//      receiveCommand();
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

