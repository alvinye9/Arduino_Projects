#include <Servo.h>

Servo top;  // create servo object to control a servo
Servo bottom;

//===================== Define Pins =================
const int topPin = 9;
const int bottomPin = 6;

const int redPin = 12;
const int greenPin = 13;

const int switchPin = 4; //for reed switch/magnetic sensor
const int openPosBottom = 2000; //for servo
const int closePosBottom = 1200;
const int openPosTop = 1000;
const int closePosTop = 1800; //for servo


const int pingPin1 = 2; //1 is the front sensor
const int echoPin1 = 3;

const int pingPin2 = 10;
const int echoPin2 = 11;

const int pingPin3 = 5;
const int echoPin3 = 7;

const int buzzPin = 4;

//===================== Define Variables =================
bool isDocked;
int buzz = 0; //piezo buzzer status (0,1,3)

long duration1; //ultrasonic sensor 1
long inches1;
long duration2; //2
long inches2;
long duration3; //3
long inches3;


//===================== Set up =================
void setup() {
  top.attach(topPin);
  bottom.attach(bottomPin);
  pinMode(topPin, OUTPUT);
  pinMode(bottomPin, OUTPUT);
  pinMode(switchPin, INPUT);

  pinMode(pingPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(pingPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(pingPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  pinMode(greenPin, OUTPUT); //green starts on
  digitalWrite(greenPin, HIGH);
  pinMode(redPin, OUTPUT); //red starts off
  digitalWrite(redPin, LOW);

  top.writeMicroseconds(closePosTop); //both servos start closed
  bottom.writeMicroseconds(closePosBottom);

  Serial.begin(115200);
}

//===================== Loooooop =================
void loop() {

  //Ultrasonic Sensor 1 (flag 1)
  digitalWrite(pingPin1, LOW);
  digitalWrite(pingPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin1, HIGH); //trigger pin 1 sends signal
  delayMicroseconds(2);
  digitalWrite(pingPin1, LOW); //trigger pin 1 is off
  duration1 = pulseIn(echoPin1, HIGH); //function returns the time it took for signal to come back
  inches1 = microsecondsToInches(duration1);

  //Ultrasonic Sensor 2 (flag 2)
  digitalWrite(pingPin2, HIGH); //trigger pin 2 sends signal
  delayMicroseconds(2);
  digitalWrite(pingPin2, LOW);//trigger pin 2 is off
  duration2 = pulseIn(echoPin2, HIGH);
  inches2 = microsecondsToInches(duration2);

  //Ultrasonic Sensor 3 (check if docked)
  digitalWrite(pingPin3, HIGH); //trigger pin 2 sends signal
  delayMicroseconds(2);
  digitalWrite(pingPin3, LOW);//trigger pin 2 is off
  duration3 = pulseIn(echoPin3, HIGH);
  inches3 = microsecondsToInches(duration3);

  /*
    Serial.print("sensor 1: ");
    Serial.print(inches1);
    Serial.println("");
    Serial.print("sensor 2: ");
    Serial.print(inches2);
    Serial.println(""); //for debugging
    Serial.print("sensor 3: ");
    Serial.print(inches3);
  */

  if (inches1 <= 5 || inches2 <= 5 || inches3 <= 8) { //if any flags or body of vehicle is detected
    isDocked = true;
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    delay(2000);
    Serial.println("Is Docked");

  }
  else {
    isDocked = false;
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
    delay(2000);
  }


  if (isDocked) { //if docked

    if (inches1 <= 5 & inches2 <= 5) { //both flags, drop 3 balls
      top.writeMicroseconds(openPosTop);
      bottom.writeMicroseconds(openPosBottom);
      Serial.println("Both Flags");
      buzz = 3;
    }
    else if (inches1 <= 5 & inches2 > 5) { //only 1 flag, drop 1 ball
      top.writeMicroseconds(closePosTop);
      bottom.writeMicroseconds(openPosBottom);
      Serial.println("One Flag");
      buzz = 1;
    }   
    else { //no flag
      top.writeMicroseconds(closePosTop);
      bottom.writeMicroseconds(closePosBottom);
      Serial.println("No Flag");
      buzz = 0;
    }
  }//end if isDocked

  else { //not docked
    top.writeMicroseconds(closePosTop);
    bottom.writeMicroseconds(closePosBottom);

    if (buzz == 1) {
      tone(buzzPin, 2000);
      delay(200);
      noTone(buzzPin);
      delay(200);
      buzz = 0;
    }
    else if (buzz == 3) {
      tone(buzzPin, 2000); //once
      delay(200);
      noTone(buzzPin);
      delay(200);
      tone(buzzPin, 2000); //twice
      delay(200);
      noTone(buzzPin);
      delay(200);
      tone(buzzPin, 2000); //thrice
      delay(200);
      noTone(buzzPin);
      delay(200);
      buzz = 0;
    }

  }//end else !isDocked




}//end looooooop


long microsecondsToInches(long microseconds) { //using the speed of sound and the time elapsed before echo pin picks up the trigger ping, calculate distance traveled
  return microseconds / 74 / 2;
}
