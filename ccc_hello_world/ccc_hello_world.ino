//
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
//
// end license header
//
// This sketch is a good place to start if you're just getting started with
// Pixy and Arduino.  This program simply prints the detected object blocks
// (including color codes) through the serial console.  It uses the Arduino's
// ICSP SPI port.  For more information go here:
//
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:hooking_up_pixy_to_a_microcontroller_-28like_an_arduino-29
//

#include <Pixy2.h>
#include <Pixy2CCC.h> //make sure to always have this

// This is the main Pixy object
Pixy2 pixy;

long largestArea = 0;
long currentArea;

void setup()
{
  Serial.begin(115200);
  Serial.print("Starting...\n");

  pixy.init();
}

void loop()
{
  int i;
  //grab blocks
  pixy.ccc.getBlocks();

  // If there are detect blocks, print them!
  if (pixy.ccc.numBlocks)
  {
    for (i = 0; i < pixy.ccc.numBlocks; i++)
    {
      currentArea = pixy.ccc.blocks[i].m_width * pixy.ccc.blocks[i].m_height;
      if ( i == 0) {
        largestArea =  pixy.ccc.blocks[0].m_width * pixy.ccc.blocks[0].m_height;
      }
      else if ( currentArea > pixy.ccc.blocks[i - 1].m_width * pixy.ccc.blocks[i - 1].m_height) {
        largestArea = currentArea;
      }
      else {
      }
    }//close for loop

    Serial.print("Largest Area: ");
    Serial.println(largestArea);

  }//end if
}//end loooop
