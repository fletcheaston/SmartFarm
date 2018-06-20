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

  New Data string per board includes NodeID and new headings for strings


-----node data-----
N3 NPROG begin //Node Reprogram command
N3 NTIME 18-3-28-20:36:00 //Node Timestamp
N3 VOLTS 4.18 3.92 //Battery and Solar Voltage
N3 SM123 1119.565 1083.424 1083.424 //Soil Moisture sensors 123
N3 ST123 -127.00 -127.00 -127.00 NA NA NA //Soil Temperature sensors 123
N3 SM456 9635.317 1936.989 5314.372 //Soil Moisture sensors 456
N3 ST456 -127.00 -127.00 -127.00 //Soil Temperature sensors 456
N3 DMECT D5 1.20 0.00 24.6 //VWC EC tp
N3 NPROG end //Node Reprogram command

  Can use the MCU com digital 6 pin to finish the upload process
  ...See Controller Firmware uploadProgram()
*/

#include <SmartFarmMeasureDAQv6_2.h> //choose v6_1 or v6_2
// updated library to include boardID on 11/13/17
//updated library to make functions return strings to save them here and first serial print, then write to sd card.
#define XBEE_RADIO_BAUD 57600

SmartFarmMeasure smf;
String BoardID = "N0";
String tStamp = ""; String voltLevels = "";
String Watermark_West = ""; String Watermark_East = "";
String Temperature_West = ""; String Temperature_East = "";

void setup() {
  Serial.begin(XBEE_RADIO_BAUD);
  String BoardID = F("N30");
  smf.finishUp();
  Wire.begin();
  
  //time and battery read functions
  tStamp = smf.timeStamp(BoardID);
  delay(1000);
  voltLevels = smf.readVolts(BoardID);
  delay(1000);

  //sensor setup functions...
  smf.setupWM();
  smf.setupTemps();

  //read functions...
  Watermark_West = smf.readWM(BoardID, 1, 2, 3); //wm1,2,3
  delay(1000);
  Temperature_West = smf.readTemps(BoardID, 1, 2, 3);
  delay(1000);
  Watermark_East = smf.readWM(BoardID, 4, 5, 6); //wm4,5,6
  delay(1000);
  Temperature_East = smf.readTemps(BoardID, 4, 5, 6);
  delay(1000);

  //print serial data section
  Serial.println(BoardID + " NPROG begin ");// prints board ID and beginning-of-transmission
  delay(1000);
  Serial.println(tStamp);// prints real-time clock data to serial port
  delay(1000);
  Serial.println(voltLevels);//prints voltage
  delay(1000);
  Serial.println(Watermark_West);//prints Watermark
  delay(1000);
  Serial.println(Temperature_West);//prints Temperature
  delay(1000);
  Serial.println(Watermark_East);//prints Watermark
  delay(1000);
  Serial.println(Temperature_East);//prints Temperature
  delay(1000);
  Serial.println(BoardID + " NPROG end ");// prints board ID and end-of-transmission
  delay(1000);
  Serial.flush();
  Serial.end();

  //wait a little bit after serial printing
  delay(2000);

  //sd write section
  smf.setupSD();
  smf.write2SD(tStamp);// writes real-time clock data to sd card
  smf.write2SD(voltLevels);//writes voltage
  smf.write2SD(Watermark_West);//writes Watermark
  smf.write2SD(Temperature_West);//writes Temperature
  smf.write2SD(Watermark_East);//writes MUX
  smf.write2SD(Temperature_East);//writes Temperature
  smf.write2SD("newline");
}

void loop() {
  // put your main code here, to run repeatedly:
}
