const int buzzPin=7;
const int forcePin=A0;
const int ledPin=6;

void setup() {
  Serial.begin(115200);
  pinMode(buzzPin,OUTPUT);
  pinMode(forcePin,INPUT);
  pinMode(ledPin,OUTPUT);
  
}

void loop() {
  int sensorVal=analogRead(forcePin);//454 to 1023
  //Serial.println(sensorVal); //for debugging
  int val=map(sensorVal, 141, 1023, 1023, 0);
  Serial.println(val); //for debugging
  analogWrite(ledPin,val);

  if(val==0){ //0N
    Serial.println("No Force Applied");
    noTone(buzzPin);
  }
  else if(val>0 && val<=941){ //0N to 4.95N
    Serial.println("Light Force Applied");
    noTone(buzzPin);
  }
  else if(val>941 && val<1023){ //4.95N to 10N
    Serial.println("3 Medium Force Applied");
    noTone(buzzPin);
  }
  else{ //10N
    Serial.println("Warning!! Heavy Force Applied");
    tone(buzzPin,250);
  }
  


}