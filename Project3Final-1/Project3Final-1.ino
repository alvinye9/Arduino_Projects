#include <Servo.h>
Servo steer;  // create servo object to control a servo
Servo throttle;
const int speedCount = 1650; //speed of throttle in ms

Servo drop;
//open and closed positions for the ball drop switch
const int openPos = 1000;
const int closePos = 2000;

// Define PINS
const int interruptPin = 2; //input
const int steerPin = 10; //output
const int throttlePin = 6; //output
const int dropPin = 11; //output
const int buzzPin = 9; //PWM output
//encoder pins
const int APin = 3; //must be an interrupt pin 2 or 3
const int BPin = 8;
//ultrasonic sensor pins
const int trigPin = 4;
const int echoPin = 7; //digital input

// Define PROPORTIONAL CONTROL variables
const float setpointDist = 24.0;
const float K_p = 1.5; //proportional gain constant

// Define INTERRUPT variables
volatile int valA;
volatile int valB;
volatile int lastStateA;


// Define ENCODER variables
int posi = 0; //experimentally determined: 700 posi= 1 meter
int distanceTraveled;

// Define condition variables
bool isSafe = true; // robot isSafe?
bool notDropped = true; // is ball notDropped?
bool isDone = false; // isDone with mission?

// Define DATA LOGGING variables
unsigned long sampleTime = 100; //time between samples
unsigned long lastTime;
unsigned long startTime;
unsigned long stopTime = 60000; //Stop after 4 seconds of logging (20 ms of slack)

//variables for timer implementations
const int delayTime = 300;
long currentTime;

void setup() {
  Serial.begin(115200);

  //Record Data
  //Mark the start of the actual log file
  Serial.println("**** Start of Log ****");
  //Let's print a header
  Serial.println("Time (ms), A0 Value");
  startTime = millis();
  lastTime = millis(); //Get the current time after void setup() is complete

  // Setup Interrupt Protocols
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonPress, FALLING);
  attachInterrupt(digitalPinToInterrupt(APin), readEncoder, CHANGE);

  // Setup all pins
  steer.attach(steerPin);
  throttle.attach(throttlePin);
  pinMode(throttlePin, OUTPUT);
  pinMode(steerPin, OUTPUT);
  steer.writeMicroseconds(1500);    //starts steer off at neutral
  throttle.writeMicroseconds(1500); //starts throttle off at neutral
  drop.attach(dropPin);
  pinMode(dropPin, OUTPUT);
  drop.writeMicroseconds(closePos); //starts drop switch off at neutral
  pinMode(APin, INPUT);
  pinMode(BPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzPin,OUTPUT);

  lastStateA = digitalRead(APin); //last state of A, used for encoder
}// end setup()



void loop() {
  //  ===================== Calculate Proportionally-Adjusted Error =====================
  int inchesToWall = returnInches();
  int inchError = returnError(inchesToWall);
  int countError = K_p * map(inchError, 0, 24, 0, -500); //MAPS ERROR COUNT TO HOW MUCH THE STEER SERVO SHOULD TURN TO COMPENSATE FOR ERROR, then multiplies by K (PID)

  // ===================== Log Data =====================
  // Every sampleTime milliseconds, call the dataLog function to
  // send the current time and A0 ADC value to the Logomatic
  // (Note, I attached A0 to 3.3V on the Arduino simply to have data to log
  // you don't have to attach anything to A0, the Arduino will just log random readings from A0
  unsigned long timeNow = millis();
  if ((timeNow - lastTime) > sampleTime) {
    dataLog(timeNow, inchesToWall);
    lastTime = timeNow;
  }
  if ((millis() - startTime) > stopTime) {
    //Serial.println("loop forever");
    while (1) {} //Loop Forever. Press reset to run the program again.

    //Press the Stop button on the logomatic v2 to write the Logomatics receive buffer to the SD card
  }
  
 // ===================== Run Robot =====================
  if (!isSafe) {

    if (distanceTraveled <= 264 ) { //Not yet done with mission (264 inches = 22 feet)
      
      if (distanceTraveled % 36 == 0 & distanceTraveled != 0 ) { //every 36 inches or 3 feet, stop and drop ball
        throttle.writeMicroseconds(1500); //stop to drop ball
        dropBall(); 
        long currentTime2 = millis();
        while (millis() - currentTime <= delayTime) { //the more precise the units of distanceTraveled are, the less this delay can be (currently 300 ms)
          throttle.writeMicroseconds(speedCount); //Tell it to keep moving so that it wont be stuck in this if-loop dropping balls
        }
      }
      
      else { // Move forward
        throttle.writeMicroseconds(speedCount);
        steer.writeMicroseconds( 1500 + (countError) ); //ADJUSTS STEER USING countError and gain constant
        // Serial.println("Running");
      }
    }

    else {// Done with mission, buzzer sounds
      throttle.writeMicroseconds(1500);
      if (!isDone) {
        playTone();
        noTone(buzzPin);
        isDone = true;
      }
      else {  // Buzzer finished sounding
        //Serial.println("done");
      }
    }

  }// end !ifSafe
  
  else { // In safe mode, don't move
    throttle.writeMicroseconds(1500);
    drop.writeMicroseconds(closePos);
  }

}//end loooooooooooop ======================================================


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
  distanceTraveled = posi / 213.3599 * 12.0 ; //converts posi to inches
}

//ISR Function for button
void buttonPress() {
  isSafe = !isSafe;
}

long microsecondsToInches(long microseconds) { //using the speed of sound and the time elapsed before echo pin picks up the trigger ping, calculate distance traveled
  return microseconds / 74 / 2;
}

int returnInches() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); //trigger pin 1 sends signal
  delayMicroseconds(2);
  digitalWrite(trigPin, LOW); //trigger pin 1 is off
  int distance = pulseIn(echoPin, HIGH); //function returns the time it took for signal to come back
  int inches = microsecondsToInches(distance);
  return inches;
}

int returnError(int currentDist) {
  int error = (setpointDist - currentDist);
  return error;
}

void dataLog(unsigned long logTime, int data) {
  Serial.println(logTime);
  Serial.println(',');
  Serial.println(data);
}

void playTone() {
  for (int i = 0; i <= 3; i++) {
    tone(buzzPin, 2000);
    delay(300);
    tone(buzzPin, 1000);
    delay(300);
    tone(buzzPin, 1500);
    delay(300);
    tone(buzzPin, 2000);
    delay(300);
    tone(buzzPin, 1000);
    delay(300);
  }
}

void dropBall() {
  long currentTime3 = millis();
  while (millis() - currentTime3 <= delayTime) {
    drop.writeMicroseconds(openPos);
  }
  long currentTime4 = millis();
  while (millis() - currentTime4 <= delayTime) {
    drop.writeMicroseconds(closePos);
  }
}
