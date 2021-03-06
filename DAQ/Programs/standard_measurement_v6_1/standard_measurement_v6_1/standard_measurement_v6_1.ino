/*
   _____          ____            __   __
  |  __ \   /\   / __ \          / /  |  |
  | |  | | /  \ | |  | | __   __/ /_  |  |
  | |  | |/ /\ \| |  | | \ \ / / '_ \ |  |
  | |__| / ____ \ |__| |  \ V /| (_) ||  |
  |_____/_/    \_\___\_\   \_/  \___(_)__|

  updated for v6.2 2/15/18
  DAQ pinout

  Comm to MCU pin D6

  D2 -- PORT A -- SDI12 -- Decagon5TE,...
  PORT B -- I2C -- RTC,...
  PORT C -- Analog -- Analog Sensors...
  D4/D5 -- PORT D -- Resistive -- Watermark200SS,...
  PORT E -- 1wire -- DS18b20
  PORT F -- Serial -- GPS,...
    SIG -- Additional Sensors

  -- For ATMEL Programming

  extended_fuses=0xFE
  high_fuses=0xDA
  low_fuses=0xE2

  Sample Text Layout, to easily parse into an excel file. This means a single data string
  Timestamp batV solV | WM1 WM2 WM3 WM4 | t1 t2 t3 t4 | Dec1: VWC EC tp Dec2: VWC EC tp Dec3: VWC EC tp
  82317_12:45 4.06v 4.53v  | NA1 NA2 NA3 NA4 | t1 t2 t3 t4 ...

  New Data string per board includes boardID
  B2 Upload  B2 110_1:13:36 B2 3.88 0.00 B2 1246.315  NA  9942.299  B2 NA NA NA NA

  Can use the MCU com digital 6 pin to finish the upload process
  ...See Controller Firmware uploadProgram()
*/

#include <SmartFarmMeasureDAQv6_1.h> //choose v6_1 or v6_2
// updated library to include boardID on 11/13/17
//updated library to make functions return strings to save them here and first serial print, then write to sd card.

SmartFarmMeasure smf;

void setup()
{
  String BoardID = "Test";
  int Temperature_Count = 2;

  smf.finishUp();
  Serial.begin(57600);

  //415AC735 coord

  Wire.begin();
  //uncomment the next line to set RTC time from compiling
  //when the battery fails this will show the default date and have to be reset

  //smf.setRTCToComputerTime(__DATE__, __TIME__);

  //sensor setup functions...
  smf.setupAll();

  Serial.println("Test");
  Serial.println(BoardID + " V " + smf.readVolts());
  Serial.println(BoardID + " W " + smf.readWM());
  Serial.println(BoardID + " T " + smf.readTemps(Temperature_Count));
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
    smf.write2SD(BoardID + " W " + smf.readWM());
    smf.write2SD(BoardID + " T " + smf.readTemps(Temperature_Count));
    smf.write2SD(BoardID + " D " + smf.readDecSensors());
    smf.write2SD("newline");
  }

}

void loop()
{
  // Nothing to loop through
}
