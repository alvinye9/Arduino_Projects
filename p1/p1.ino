//CDT Alvin Ye
//Company B2
//Section L2, LTC Heyman
//Project 1

#include <Servo.h>

Servo steer;  // create servo object to control a servo
Servo throttle;

//===================== Define Pins =================
const int steerPin = 10;
const int throttlePin = 11;
const int safetyPin = 5;
const int searchPin = 4;
const int potpin = A3;
const int greenPin = 3;
const int redPin = 2;

//===================== Define Variables =================
int val;    // variable to read for throttle/speed
int reading; //variable to read the safety button
int reading1; //variable to read the execute search button

//===================== Define Initial Conditions =================
bool isSafe = true; //is the robot in safe mode?
bool isSearching = false; //is the robot searching?
bool ledBlink = true; //controls blinking sequence of leds in safe mode

int buttonState = HIGH;           // the current state of safety button
int lastButtonState = HIGH;   // the previous reading from safety button (starts HIGH)
unsigned long lastDebounceTime = 0;  // the last time the safety button was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

unsigned long tPrevious = 0; //the last time led blinked (used to alternate blinking)
unsigned long tCurrent;

int buttonState1 = HIGH; // the current state of search button
int lastButtonState1 = HIGH;
unsigned long lastDebounceTime1 = 0;
unsigned long debounceDelay1 = 50;

const int oneSec = 725;
const int twoSec = 1450; //approx # of time Arduino runs loop() in 2 seconds including print statements
unsigned long t1 = twoSec; //seq1 of search
unsigned long t2 = oneSec; //seq2 of search
unsigned long t3 = oneSec; //seq3 of search
bool isDone1 = false; //is seq1 done?
bool isDone2 = false; //is seq2 done?
bool isDone3 = false; //is seq3 done?

//===================== Set up =================
void setup() {
  steer.attach(steerPin);  // attaches the steering servo on pin 10 to the servo object
  throttle.attach(throttlePin);
  pinMode(throttlePin, OUTPUT);
  pinMode(steerPin, OUTPUT);

  pinMode(safetyPin, INPUT_PULLUP); //inverts logic of pin (and button) to save a resistor
  pinMode(searchPin, INPUT_PULLUP); //or two

  pinMode(greenPin, OUTPUT);
  digitalWrite(greenPin, LOW);
  pinMode(redPin, OUTPUT);
  digitalWrite(redPin, LOW);

  Serial.begin(115200);
}

//===================== Loooooop =================
void loop() {
  val = analogRead(potpin);
  val = map(val, 0, 1023, 1000, 2000);

  int reading = digitalRead(safetyPin);
  int reading1 = digitalRead(searchPin);

  //===================== Debouncing Safety Button =================
  // Part of code retrieved from Arduino's "debounce" example file

  if (reading != lastButtonState) {   // If the switch changed, due to noise or pressing:
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    if (reading != buttonState) {    // if the button state has changed:
      buttonState = reading;

      if (buttonState == HIGH) {//button is pressed
        isSafe = !isSafe;
        Serial.println("Toggled Safety");
      }
    }
  }


  //================= Safe or Armed Mode ================
  if (isSafe) {  //safe mode
    throttle.writeMicroseconds(1500);  
    isDone1=false;
    isDone2=false;
    isDone3=false;
    t1=twoSec;
    t2=oneSec;
    t3=oneSec;
    isSearching=false;
    
    tCurrent = millis();
    if (tCurrent - tPrevious > 200) {//every 0.2 sec
      if (ledBlink) {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, HIGH);
      }
      else {
        digitalWrite(greenPin, LOW);
        digitalWrite(redPin, HIGH);
      }
      ledBlink = !ledBlink; //alternates blink sequence next loop
      tPrevious = millis();
    }
    //Serial.println("safe"); //for debugging
    
  }//end if isSafe

  else {     //armed mode
    //================= Debouncing Execute Search Button ================
    if (reading1 != lastButtonState1) {
      lastDebounceTime1 = millis();
    }

    if ((millis() - lastDebounceTime1) > debounceDelay1) {

      // if the button state has changed:
      if (reading1 != buttonState1) {
        buttonState1 = reading1;

        if (buttonState1 == HIGH) {
          Serial.println("Toggled Search");
          isSearching = !isSearching;
        }
      }
    }//end debouncing search button

    //================ Controlling LEDs =================
    if (val < 1500) { //moving backwards
      //Serial.println("backwards");
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
    }
    else { //moving forwards
      //Serial.println("forwards");
      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, LOW);
    }
    
    //================ Searching Sequence =================
    if (isSearching ) {
      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, LOW);

      if (isDone1 == false) { //Sequence 1
        Serial.println("Searching Seq 1");
        //t1 = millis();
        if ( t1 >= 1 ) { //for 2 seconds, >=1 b/c unsigned ints can't be negative
          throttle.writeMicroseconds(1750);
          //Serial.println("sequence 1.1"); //for debugging
          t1 = t1 - 1;
        }
        else { //Sequence 1 is complete
          isDone1 = true;
          steer.writeMicroseconds(1750);//half right face
          //Serial.println("sequence 1.2"); //for debugging
        }
      }

      else if (isDone1 == true && isDone2 == false) { //Sequence 2
        Serial.println("Searching Seq 2");
        if ( t2 >= 1) { //for 1 second
          throttle.writeMicroseconds(1750); //half speed
          //Serial.println("sequence 2.1"); //for debugging
          t2 = t2 - 1;
        }
        else {
          isDone2 = true;
          steer.writeMicroseconds(1250);//half left face
          //Serial.println("sequence 2.2"); //for debugging
        }
      }

      else if (isDone1 == true && isDone2 == true && isDone3 == false) { //Seq 3
        Serial.println("Searching Seq 3");
        if ( t3 >= 1 ) { //for 1 second
          throttle.writeMicroseconds(1750); 
          //Serial.println("Sequence 3");
          t3 = t3 - 1;
        }
        else {
          isDone3 = true;
        }
      }
      else {
        steer.writeMicroseconds(1500);
        isSafe = true; //return to safe mode after sequence 3 is done
        isSearching = false; 
        
        isDone1 = false; //reset search mode variables
        isDone2 = false;
        isDone3 = false;
        t1 = twoSec;
        t2 = oneSec;
        t3 = oneSec;
        Serial.println("Searching Complete");
      }
    }//end if isSearching
    
    else { //if not actively searching, reset all search parameters
      throttle.writeMicroseconds(val);
      steer.writeMicroseconds(1500);
      isDone1 = false;
      isDone2 = false;
      isDone3 = false;
      t1=twoSec;
      t2=oneSec;
      t3=oneSec;
    }//end else not isSearching

    
  }//end else armed
  
  lastButtonState = reading; //update button states
  lastButtonState1 = reading1;
}
