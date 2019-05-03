
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
    String BoardID = "Test 9";

    smf.finishUp();
    Serial.begin(57600);
    Wire.begin();

    smf.setupAll();

    Serial.println(BoardID + " " + smf.readVolts());

    Serial.flush();
    Serial.end();
    delay(2000);

}

void loop()
{
    //Nothing to loop through
}
