#include <Servo.h>
Servo pivot;
Servo steer;
Servo throttle;
Servo trigger;

#define I2C
#define X_CENTER (pixy.frameWidth / 2)

#include <Pixy2I2C.h>
Pixy2I2C pixyCCC;

#include <Pixy2.h>
Pixy2 pixy;

const int ENEMY = 11;
const int FRIENDLY = 19;
const int MOVING = 715;
const int MIDDLE = 1500;
const int LEFT = 2000;
const int RIGHT = 1000;

const int steerPin = 5;     //output
const int throttlePin = 4;  //output
const int pivotPin = 44;    //PWM
const int triggerPin = 7;  //PWM
const int buzzerPin = 46;   //PWM

//Variables for line following PID
int setpoint = 39;
int error = 0;
int sumError = 0;
int lastError = 0;
int Kp = 50;
int Kd = 10;
int Ki = 0;

//Variables for aiming PID
int setpoint_aim = 150;
int error_aim = 0;
int sumError_aim = 0;
int lastError_aim = 0;
int Kp_aim = 1;
int Kd_aim = 0.4;
int Ki_aim = 1;

bool isStop = false;       //used to enter/exit the if detect vector loop
bool isObjective = false;  //used to enter/exit the if detect target loop


const long speed = 1700;


void setup() {
  Serial.begin(115200);
  
  //Serial.print("Starting...\n");

  trigger.attach(triggerPin);
  trigger.writeMicroseconds(1460);
  pivot.attach(pivotPin);
  pivot.writeMicroseconds(MIDDLE);

  steer.attach(steerPin);
  throttle.attach(throttlePin);
  throttle.writeMicroseconds(1500);
  steer.writeMicroseconds(1500);

  pinMode(buzzerPin, OUTPUT);

  pixyCCC.init();
  pixyCCC.setLamp(1,1);
  pixy.init();
  pixy.setLamp(1, 1);
}

void loop() {
  
  //====================== LINE FOLLOWING MODE ==============================
  int8_t res;
  char buf[96];
  static long initialTime = millis();

  Serial.print(error_aim);
  Serial.print(",");
  Serial.println(millis() - initialTime);

  // Get latest data from Pixy, including main vector, new intersections and new barcodes.
  res = pixy.line.getMainFeatures();

  // If error or nothing detected, stop motors
  if (res <= 0) {
    throttle.writeMicroseconds(1500);
    //steer.writeMicroseconds(1500);

    //Serial.print("NOTHING DETECTED");  //UNCOMMENT
    //Serial.println(res);
    //return;
  }

  // If it sees a vector
  if (res & LINE_VECTOR && !isStop) {
    //pixy.line.vectors->print();

    // Run PID for line following
    PIDcontrol();

    // If vector is heading away from us (arrow pointing up), things are normal.
    if (pixy.line.vectors->m_y0 > pixy.line.vectors->m_y1) {
      // ... but slow down a little if intersection is present, so we don't miss it.
      if (pixy.line.vectors->m_flags) {
        throttle.writeMicroseconds(speed - 0);
        //Serial.println("INTERSECTION: slow down");  //UNCOMMENT
      } else                                        // otherwise, pedal to the metal!
      {
        throttle.writeMicroseconds(speed);
        //Serial.println("PEDAL TO THE METAL");  //UNCOMMENT
      }
    } else  // If the vector is pointing down, or down-ish, we need to go backwards to follow.
    {
      throttle.writeMicroseconds(speed - 200);
      //Serial.println("BACKWARDS");  //UNCOMMENT
    }
    PIDcontrol();
  }//end if found vector (moving)
  else {  //no vector detected
    //Serial.println("STOPPED");
    throttle.writeMicroseconds(1500);
  }

//Serial.println(isStop);
  /*
  // If intersection, do nothing (we've already set the turn)
  if (res & LINE_INTERSECTION) {
    //pixy.line.intersections->print();
    //Serial.println("INTERSECTION");
  }
*/

  if (res & LINE_BARCODE) {  //if detects barcode
    //pixy.line.barcodes->print();
    // code==0 is our stop signal
    if (pixy.line.barcodes->m_code == 0) {
      throttle.writeMicroseconds(1500);
      isStop = true;
      //Serial.print("STOP BARCODE");  //UNCOMMENT
      exit(0);
    }
    // code==1 is our left-turn sign
    else if (pixy.line.barcodes->m_code == 1) {
      pixy.line.setNextTurn(60);        // 60 degrees is a left turn
     // Serial.print("Next Turn: LEFT");  //UNCOMMENT
    }
    // code==2 is our right-turn sign
    else if (pixy.line.barcodes->m_code == 2) {
      pixy.line.setNextTurn(-60);        // -60 is a right turn
     // Serial.print("Next Turn: RIGHT");  //UNCOMMENT
    } else if (pixy.line.barcodes->m_code == 3) {
      pixy.line.setNextTurn(0);
      //Serial.print("Next Turn: STRAIGHT");         //UNCOMMENT
    } else if (pixy.line.barcodes->m_code == 4) {  //detects objective
      isObjective = true;
      isStop = true;
      throttle.writeMicroseconds(1500);
      steer.writeMicroseconds(1500);
      //Serial.println("OBJECTIVE");  //UNCOMMENT
    } else if (pixy.line.barcodes->m_code == 5) {
      isObjective = true;
      isStop = true;
      throttle.writeMicroseconds(1500);
      steer.writeMicroseconds(1500);
      //Serial.print("BONUS"); //UNCOMMENT
    }
  }


  //=============== COLOR CONNECTED COMPONBENTS MODE (AIMING MODE) ===============
  // grab blocks!
  pixyCCC.ccc.getBlocks();

  if (pixyCCC.ccc.numBlocks) {  //If detects target
    //Serial.print("DETECTED TARGET");
    //Serial.println(pixyCCC.ccc.blocks[0].m_signature);
    //if (isObjective == true) {  //if it has previously detected an objective barcode
      if (pixyCCC.ccc.blocks[0].m_signature == FRIENDLY) {
        pivot.writeMicroseconds(MIDDLE);
        playTune();
        isStop = false;
        isObjective = false;
        //Serial.println("FRIENDLY DETECTED");
        // Serial.println(pixyCCC.ccc.blocks[0].m_x);
      } else if (pixyCCC.ccc.blocks[0].m_signature == 11) {
       // Serial.print("ENEMY DETECTED: ");
        throttle.writeMicroseconds(1500);
        steer.writeMicroseconds(1500);
        PIDaim();

        // Serial.println(pixyCCC.ccc.blocks[0].m_x);

      } else if (pixyCCC.ccc.blocks[0].m_signature == MOVING) {
        //Serial.print("MOVING ENEMY DETECTED:");
        throttle.writeMicroseconds(1500);
        steer.writeMicroseconds(1500);
        PIDaimMoving();

        //Serial.println(pixyCCC.ccc.blocks[0].m_x);
      }
    //} else {}  //if it has not previously seen an objective barcode
    
  }
  else{ //Doesn't detect target (anymore)
      //Serial.println("NO TARGET");
      pivot.writeMicroseconds(MIDDLE);
      isObjective = false;
      isStop = false;
  }

}  //end loooooooooooooooooooooooooooooop




//==================== FUNCTIONS ====================

void PIDcontrol() {
  //throttle.writeMicroseconds(speed);
  error = (int32_t)pixy.line.vectors->m_x1 - (int32_t)X_CENTER;
  //Serial.println(error);
  sumError = sumError + error;
  long steerInput = 1500 + (Kp * error + Kd * (error - lastError) + (Ki * sumError));
  if (steerInput < 1000) {
    steerInput = 1000;
  } else if (steerInput > 2000) {
    steerInput = 2000;
  }

  // Serial.println(steerInput);
  steer.writeMicroseconds(steerInput);
  lastError = error;
  //Serial.println(error);
}


void PIDaim() {
  error_aim = 160 - pixyCCC.ccc.blocks[0].m_x;  //150 is the center frame location
  //Serial.println(error_aim);
  sumError_aim = sumError_aim + error_aim;
  long pivotInput = MIDDLE + (Kp_aim * error_aim) + Kd_aim * (error_aim + lastError_aim) + (Ki_aim * sumError_aim);
  //Serial.println(pivotInput);
  delay(200);

  if (pivotInput < 700) {
    pivotInput = 700;
    sumError_aim = sumError_aim - error_aim;
  } else if (pivotInput > 2200) {
    pivotInput = 2200;
    sumError = sumError - error_aim;
  }

  // Serial.println(steerInput);
  pivot.writeMicroseconds(pivotInput);
  lastError_aim = error_aim;
  //Serial.println(pivotInput);

  if(abs(error_aim) < 5){
  shootTarget();
  }
}

void PIDaimMoving() {
  error_aim = 160 - pixyCCC.ccc.blocks[0].m_x;  //150 is the center frame location
  //Serial.println(error_aim);
  sumError_aim = sumError_aim + error_aim;
  long pivotInput = MIDDLE + (Kp_aim * error_aim) + Kd_aim * (error_aim + lastError_aim) + (Ki_aim * sumError_aim);
  //Serial.println(pivotInput);
  delay(100);

  if (pivotInput < 700) {
    pivotInput = 700;
    sumError_aim = sumError_aim - error_aim;
  } else if (pivotInput > 2200) {
    pivotInput = 2200;
    sumError = sumError - error_aim;
  }

  // Serial.println(steerInput);
  pivot.writeMicroseconds(pivotInput);
  lastError_aim = error_aim;
  //Serial.println(pivotInput);

  if(abs(error_aim) < 20){
  shootTarget();
  }
}




void shootTarget() {
const int SAFE = 200;
const int FIRE = 2000;
const int TRIGGERDELAY = 1000;
  
  //Serial.println("PULL TRIGGER ONCE1");
  trigger.writeMicroseconds(2200);
  delay(TRIGGERDELAY);
 // Serial.println("STOPPP");
  delay(TRIGGERDELAY);
  trigger.writeMicroseconds(500);
  //delay(TRIGGERDELAY);
  delay(500);
  trigger.writeMicroseconds(1460);
  delay(500);

}

void playTune() {
  tone(buzzerPin, 700);
  delay(500);
  tone(buzzerPin, 200);
  delay(500);
  tone(buzzerPin, 1000);
  delay(500);
  tone(buzzerPin, 500);
  delay(500);
  noTone(buzzerPin);
}