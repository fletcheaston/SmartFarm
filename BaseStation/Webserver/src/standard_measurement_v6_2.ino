/*
   _____          ____            __   ___
  |  __ \   /\   / __ \          / /  |__ \
  | |  | | /  \ | |  | | __   __/ /_     ) |
  | |  | |/ /\ \| |  | | \ \ / / '_ \   / /
  | |__| / ____ \ |__| |  \ V /| (_) | / /_
  |_____/_/    \_\___\_\   \_/  \___(_)____|

*/

#include <SmartFarmMeasureDAQv6_2.h> //choose v6_1 or v6_2
// updated library to include boardID on 11/13/17
//updated library to make functions return strings to save them here and first serial print, then write to sd card.

SmartFarmMeasure smf;

void setup()
{
  String BoardID = "FL";
  
  smf.finishUp();
  Serial.begin(57600);
  Wire.begin();

  //sensor setup functions...
  smf.setupAll();

  Serial.println(BoardID + " V " + smf.readVolts());
  Serial.println(BoardID + " W1 " + smf.readWM(1));
  Serial.println(BoardID + " W2 " + smf.readWM(2));
  Serial.println(BoardID + " W3 " + smf.readWM(3));
  Serial.println(BoardID + " W4 " + smf.readWM(4));
  Serial.println(BoardID + " W5 " + smf.readWM(5));
  Serial.println(BoardID + " W6 " + smf.readWM(6));
  Serial.println(BoardID + " W7 " + smf.readWM(7));
  Serial.println(BoardID + " W8 " + smf.readWM(8));
  Serial.println(BoardID + " T " + smf.readTemps());
  Serial.println(BoardID + " D " + smf.readDecSensors());
  Serial.flush();
  Serial.end();
  //wait a little bit after serial printings
  delay(2000);

  if(smf.checkSafeSDVolts())
  {
    //sd write section
    smf.setupSD();
    smf.write2SD(BoardID + " TS " + smf.timeStamp());
    smf.write2SD(BoardID + " V " + smf.readVolts());
    Serial.println(BoardID + " W1 " + smf.readWM(1));
    Serial.println(BoardID + " W2 " + smf.readWM(2));
    Serial.println(BoardID + " W3 " + smf.readWM(3));
    Serial.println(BoardID + " W4 " + smf.readWM(4));
    Serial.println(BoardID + " W5 " + smf.readWM(5));
    Serial.println(BoardID + " W6 " + smf.readWM(6));
    Serial.println(BoardID + " W7 " + smf.readWM(7));
    Serial.println(BoardID + " W8 " + smf.readWM(8));
    smf.write2SD(BoardID + " T " + smf.readTemps());
    smf.write2SD(BoardID + " D " + smf.readDecSensors());
    smf.write2SD("newline");
  }

}

void loop()
{
  // Nothing to loop through
}
