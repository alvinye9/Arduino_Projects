#include <Servo.h>
#include <Pixy2.h>
#include <Pixy2CCC.h> //make sure to always have this

// create main Pixy object
Pixy2 pixy;

// create Servo objects
Servo steer;
Servo throttle;

// Pins for steering and driving control---------------------------
const int steerPin = 10; //output
const int throttlePin = 6; //output

// Variables for distance control----------------------------------
const int targetHeight = 50; // experimentally calibrated for distance of 3ft with bottom of pink sheet flush with bottom of metal plate
const int maxHeight = 206; //experimentally determined maximum pixel height pixy2 can read
const float Kp_throt = 1.3;
const float Ki_throt = 1; //probably not needed
const float Kd_throt = 7;

// Variables for turning control-----------------------------------
const int targetX = 158;
const int maxX = 316;
const int minTargetLocation;
const float Kp_steer = 1.80;
const float Ki_steer = 1; //Probably not needed
const float Kd_steer = 10;
int sumErrorX = 0;
int lastErrorX = 0;

long currentArea; //if using area instead of height

int sumError = 0; //for PI control
int lastError = 0; //for PD control

// ================ SETUP ================
void setup() {
  Serial.begin(115200);
  steer.attach(steerPin);
  throttle.attach(throttlePin);
  pinMode(throttlePin, OUTPUT);
  pinMode(steerPin, OUTPUT);
  steer.writeMicroseconds(1500);    //starts steer off at neutral
  throttle.writeMicroseconds(1500); //starts throttle off at neutral

  pixy.init();
  delay(20);
}

// ================ LOOOOOP ================
void loop() {
  
  //grab blocks
  pixy.ccc.getBlocks();

  unsigned long currentTime = millis();

  int largestHeight = driveControl();

  int xPosition = steerControl();
  
  dataLog(currentTime, xPosition, largestHeight);
}// end LOOOOOOP

void driveControl() {
  if (pixy.ccc.numBlocks) { //if blocks are detected then move

    int largestHeight = pixy.ccc.blocks[0].m_height; //pixy.ccc.blocks[0].m_width could also be used to multiply for area but height has less fluctuation
    int errorHeight = targetHeight - largestHeight; //used for Proportional control
    int mappedError = map(errorHeight, targetHeight - maxHeight, targetHeight, -500, 500); //tech not necessary but makes tuning easier
    sumError = sumError + mappedError; //used for Integral control

    //long throttleInput = 1500 +  Kp_throt*mappedError ; //P control
    long throttleInput = 1500 +  Kp_throt * mappedError + Kd_throt * (mappedError - lastError); //PD control
    //long throttleInput = 1500 +  Kp_throt*mappedError + Ki_throt*sumError + Kd_throt*(mappedError-lastError); //PID control (NOT NEEDED)
    if (throttleInput < 1000) {
      throttleInput = 1000;
    }
    else if (throttleInput > 2000) {
      throttleInput = 2000;
    }
  
    throttle.writeMicroseconds(throttleInput);
    lastError = mappedError; //update lastError
  }

  else { // if no blocks detected then don't move
    //delay(20); //don't include delay unless robot is spazzing out
    //throttle.writeMicroseconds(1500);
    //Serial.println("No blocks detected");
  }
}

void steerControl() {
if (pixy.ccc.numBlocks) {
    //Serial.println("I got here!");
    int xPosition = pixy.ccc.blocks[0].m_x;
    //Serial.println(xPosition);
    int errorX = targetX - xPosition;
    int mappedErrorX = map(errorX, targetX - maxX, targetX, 500, -500);

    sumErrorX = sumErrorX + mappedErrorX;

    long steerInput = 1500 + Kp_steer * mappedErrorX + Kd_steer * (mappedErrorX - lastErrorX);
    if (steerInput < 1000) {
      steerInput = 1000;
    }
    else if (steerInput > 2000) {
      steerInput = 2000;
    }
    Serial.println(steerInput);
    steer.writeMicroseconds(steerInput);

    lastErrorX = mappedErrorX;
  }

  //static long lastValues[numValuesInMovingAverageArray] = {0.0};
}

void dataLog(unsigned long logTime, int xPosition, int largestHeight) {
  Serial.print(logTime);
  Serial.print(',');
  Serial.print(xPosition);
  Serial.print(',');
  Serial.println(largestHeight);
}
