// DO NOT change numBlocks, blength, or bwidth
const int numBlocks=5;
// Arrays that contain the length and width values of numBlocks rectangles
int blength[numBlocks]={1,5,500,300,20};
int bwidth[numBlocks]={100,200,208,1000,5};

// DO NOT change numBlocks, blength, or bwidth

void setup() {
  long Area[numBlocks]; //array of longs
  Serial.begin(115200);
  //Calculate and display the area of each rectangle
  for(int i=0;i<numBlocks;i++){
    Area[i]=long(blength[i])*long(bwidth[i]); //typecast all values to long
    Serial.println(Area[i]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
