/*
  _____          ____            __   ___
  |  __ \   /\   / __ \          / /  |__ \
  | |  | | /  \ | |  | | __   __/ /_     ) |
  | |  | |/ /\ \| |  | | \ \ / / '_ \   / /
  | |__| / ____ \ |__| |  \ V /| (_) | / /_
  |_____/_/    \_\___\_\   \_/  \___(_)____|

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

#include <SmartFarmMeasureDAQv6_2.h> //choose v6_1 or v6_2
// updated library to include boardID on 11/13/17
//updated library to make functions return strings to save them here and first serial print, then write to sd card.

SmartFarmMeasure smf;
String BoardID = "B3";
String tStamp = ""; String voltLevels = "";
String Watermark = "";
String Temperature_West = ""; String Temperature_East = "";

void setup() {
  smf.finishUp();

  Wire.begin();
  //uncomment the next line to set RTC time from compiling
  //when the battery fails this will show the default date and have to be reset
  
 //smf.setRTCToComputerTime(__DATE__, __TIME__);
  
  //time and battery read functions
  tStamp = smf.timeStamp(BoardID);
  delay(1000);
  voltLevels = smf.readVolts(BoardID);
  delay(1000);

  //sensor setup functions...
  smf.setupWM();
  smf.setupTemps();

  //read functions...
  Watermark = smf.readWM_West(BoardID);//wm1,2,3
  delay(1000);
  Temperature_West = smf.readTemps(BoardID, 1, 2, 3);
  delay(1000);
  Temperature_East = smf.readTemps(BoardID, 4, 5, 6);
  delay(1000);

  //print serial data section
  Serial.begin(57600);
  Serial.println(BoardID + " Upload ");// prints board ID and wireless programming upload on next wake
  delay(1000);
  Serial.println(tStamp);// prints real-time clock data to serial port
  delay(1000);
  Serial.println(voltLevels);//prints voltage
  delay(1000);
  Serial.println(Watermark);//prints Watermark
  delay(1000);
  Serial.println(Temperature_West);//prints Temperature
  delay(1000);
  Serial.println(Temperature_East);//prints Temperature
  delay(1000);
  Serial.flush();
  Serial.end();

  //wait a little bit after serial printing
  delay(2000);

  //sd write section
  smf.setupSD();
  smf.write2SD(tStamp);// writes real-time clock data to sd card
  smf.write2SD(voltLevels);//writes voltage
  smf.write2SD(Watermark);//writes Watermark
  smf.write2SD(Temperature_West);//writes Temperature
  smf.write2SD(Temperature_East);//writes Temperature
  smf.write2SD("newline");
}

void loop() {
  // put your main code here, to run repeatedly:
}
