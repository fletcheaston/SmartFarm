
/*
     _____          ____            __   ___
    |  __ \   /\   / __ \          / /  |__ \
    | |  | | /  \ | |  | | __   __/ /_     ) |
    | |  | |/ /\ \| |  | | \ \ / / '_ \   / /
    | |__| / ____ \ |__| |  \ V /| (_) | / /_
    |_____/_/    \_\___\_\   \_/  \___(_)____|

*/

#include <SmartFarmMeasureDAQv6_2.h>

SmartFarmMeasure smf;

void setup()
{
    String BoardID = "A";

    smf.finishUp();
    Serial.begin(57600);
    Wire.begin();

    smf.setupAll();

    Serial.println(BoardID + " V " + smf.readVolts());
    Serial.println(BoardID + " A " + smf.readDecSensors());
    Serial.println(BoardID + " D1 " + smf.readWM(1));
    Serial.println(BoardID + " D2 " + smf.readWM(2));
    Serial.println(BoardID + " D3 " + smf.readWM(3));
    Serial.println(BoardID + " D4 " + smf.readWM(4));
    Serial.println(BoardID + " D5 " + smf.readWM(5));
    Serial.println(BoardID + " D6 " + smf.readWM(6));
    Serial.println(BoardID + " D7 " + smf.readWM(7));
    Serial.println(BoardID + " D8 " + smf.readWM(8));
    Serial.println(BoardID + " E " + smf.readTemps());

    Serial.flush();
    Serial.end();
    delay(2000);

}

void loop()
{
    //Nothing to loop through
}
