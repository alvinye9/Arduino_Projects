#include <Servo.h>

Servo steer;
Servo throttle;

const int APin = 2; //must be an interrupt pin 2 or 3
const int BPin = 8;
const int interruptPin = 3;
const float setpoint = 60.0; // [in/s], 5 [ft/s]
float sumError = 0;

const int Kp = 9;
const int Kd = 3;
const int Ki = 0.5; //change if I-control is needed
int lastPosi = 0;

bool isRunning = false;

volatile int posi = 0;
volatile float distanceTraveled = 0;
volatile float lastDistanceTraveled = 0;
volatile int currentVel = 0; //updated in ISR
int lastError = 0;

// Throttle and Steer Pins
const int steerPin = 10; //output
const int throttlePin = 6; //output

// Define INTERRUPT variables
volatile int valA;
volatile int valB;
volatile int lastStateA;

volatile int count = 0;//count interrupt Button Presses, for debugging

// Variables for BONUS
volatile long lastTime = millis();




void setup() {
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(APin), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonPress, FALLING);
  steer.attach(steerPin);
  throttle.attach(throttlePin);

  pinMode(throttlePin, OUTPUT);
  pinMode(steerPin, OUTPUT);
  pinMode(APin, INPUT);
  pinMode(BPin, INPUT);

  steer.writeMicroseconds(1500);
  throttle.writeMicroseconds(1500);
}

void loop() {
  static long initialTime = millis();//for datalogging

  if (isRunning) {
    float error = setpoint - currentVel;
    float sumError = sumError - lastError;
    int throttleInput = 1500 + (Kp * error + Kd * (error - lastError) + Ki * sumError);
    if (throttleInput > 2000) {
      throttleInput = 2000;
    }
    else if (throttleInput < 1000) {
      throttleInput = 1000;
    }
    throttle.writeMicroseconds(throttleInput);
    sumError = sumError - lastError;
    lastError = error;

/* //Printing values for datalogging
    Serial.print(currentVel);
    Serial.print("'");
    Serial.println(millis()-initialTime);
  */  
    steer.writeMicroseconds(1400); //keeps it steering straight
  }

  else { //not running
    steer.writeMicroseconds(1400);
    throttle.writeMicroseconds(1500);
    initialTime = millis();
    //Serial.println("Not Running"); //for debugging
  }
  //Serial.println(count); //for debugging

}//end looooop


//ISR Function for encoder
void readEncoder() {
  valB = digitalRead(BPin);
  valA = digitalRead(APin);
  if ( valA != lastStateA) {
    if (digitalRead(BPin) != valA) {
      posi++;
    }
    else {
      posi--;
    }
  }
  lastStateA = valA;
  //distanceTraveled = posi / 213.3599 * 12.0 ; //converts posi to inches
  distanceTraveled = posi / 220.0 * 12.0 ; //converts posi to inches, 220 pos = 1 ft

  // Code to measure velocity (every 0.1 s)
  if ( (millis() - lastTime) > 100) { //every 0.1 sec
    currentVel = (distanceTraveled - lastDistanceTraveled)*10.0; /// 220.0 * 12.0  * 10.0;
    lastDistanceTraveled = distanceTraveled;
    lastTime = millis();
  }
}

//function to be run when interrupt triggered
void buttonPress() {
  ++count; //increments and returns count, for debugging
  isRunning = !isRunning;
  posi = 0;
  lastError = 0;
  currentVel = 0;
  sumError = 0;
}
