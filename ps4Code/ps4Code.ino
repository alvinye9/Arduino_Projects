#include <Servo.h>
#include <Pixy2.h>
#include <Pixy2CCC.h> //make sure to always have this

// create main Pixy object
Pixy2 pixy;

// create Servo objects
Servo steer;
Servo throttle;

const int steerPin = 10; //output
const int throttlePin = 6; //output


const int targetHeight = 50; // experimentally calibrated for distance of 3ft with bottom of pink sheet flush with bottom of metal plate
const int maxHeight = 206; //experimentally determined maximum pixel height pixy2 can read
const float Kp_throt = 1.85;
const float Ki_throt = 1; //probably not needed
const float Kd_throt = 5;

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
    Serial.println(throttleInput);
    throttle.writeMicroseconds(throttleInput);

    //Serial.println(largestHeight);

    lastError = mappedError; //update lastError
  }

  else { // if no blocks detected then don't move
    //delay(20); //don't include delay unless robot is spazzing out
    throttle.writeMicroseconds(1500);
    Serial.println("No blocks detected");
  }




 
}// end LOOOOOOP
