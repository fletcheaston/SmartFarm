#include <SmartFarmMeasure.h>

SmartFarmMeasure smf;

void setup() {
  // put your setup code here, to run once:
  smf.setupAll();
  smf.runAll();
  smf.printUpload();
}

void loop() {
  // put your main code here, to run repeatedly:

}
