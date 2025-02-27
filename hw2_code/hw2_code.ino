float val; //for debugging
float V_out; //for debugging
int vPin = A0; //for debugging
int interruptPin = 2;
int count = 0;

void setup()
{
  Serial.begin(115200);
  //interrupt when pin 2 goes from HI to LO
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonPress, FALLING);
}

void loop()
{
  /*
  //for debugging
  val = analogRead(vPin);
  V_out = val*(5.0/1023.0);
  Serial.println(V_out); 
  */
  delay(1000);
  Serial.println(count);
  
}

//function to be run when interrupt triggered
void buttonPress(){ 
  ++count; //increments and returns count
}
