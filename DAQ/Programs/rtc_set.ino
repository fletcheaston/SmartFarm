/*
   _____          ____            __   
  |  __ \   /\   / __ \          / /  
  | |  | | /  \ | |  | | __   __/ /_  
  | |  | |/ /\ \| |  | | \ \ / / '_ \   
  | |__| / ____ \ |__| |  \ V /| (_) | 
  |_____/_/    \_\___\_\   \_/  \___/

  >>>>>works for v6.1 & v6.2, not tested on v5.4, but it should work as this is just a software change. May need to edit the pins though for v5.4.

  updated library to include boardID on 11/13/17
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
  // updated library to include boardID on 11/13/17
  //updated library to make functions return strings to save them here and first serial print, then write to sd card.

*/

#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68 //Defined RTC Address
#define Coms2MCU 6
#define XBEE_RADIO_BAUD 57600


String BoardID = "N0";
String tStamp = "";
String M = "";
String D = "";
String Y = "";
String HR = "";
String MIN = "";
String S = ""; //strings for RTC



void setup() {
  finishUp();
  Wire.begin();
  //uncomment the next line to set RTC time from compiling
  //when the battery fails this will show the default date and have to be reset
  setRTCToComputerTime(__DATE__, __TIME__);

  Serial.begin(XBEE_RADIO_BAUD);
  tStamp = timeStamp(BoardID);
  delay(1000);
  Serial.println(tStamp);// prints real-time clock data to serial port
  delay(1000);

}



void loop() {
  // put your main code here, to run repeatedly:

}


// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}



void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year) {
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}



//this function sets the RTC time from the compile time
void setRTCToComputerTime(char myDATEString[], char myTIMEString[])
{
  byte s, MIN, h, dW, d, m, y;
  s = atoi(&myTIMEString[6]);
  //Serial.println(s);
  MIN = atoi(&myTIMEString[3]);
  //Serial.println(MIN);
  h = atoi(&myTIMEString[0]);
  //Serial.println(h);
  dW = 0; // don't care about the day of week
  d = atoi(&myDATEString[4]);
  //Serial.println(d);
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
  switch (myDATEString[0]) {
    //condition date[1] == 'a' is true, return 1, elseif m = date[2] == 'n' ? 6 : 7;
    case 'J': m = myDATEString[1] == 'a' ? 1 : m = myDATEString[2] == 'n' ? 6 : 7; break;
    case 'F': m = 2; break;
    //ternary statement    if date[2] == 'r' TRUE then return 4, else return 8
    case 'A': m = myDATEString[2] == 'r' ? 4 : 8; break;
    case 'M': m = myDATEString[2] == 'r' ? 3 : 5; break;
    case 'S': m = 9; break;
    case 'O': m = 10; break;
    case 'N': m = 11; break;
    case 'D': m = 12; break;
  }
  //Serial.println(m);
  y = atoi(&myDATEString[9]);
  //Serial.println(y);
  setDS3231time(s, MIN, h, dW, d, m, y);
}


void readDS3231time(byte *second,//RTC function
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}



String timeStamp(String boardID) //RTC function
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  String Text = boardID;
  Text += " NTIME ";
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
                 &year);
  // send it to the serial monitor
  M = String(month, DEC);
  D = String(dayOfMonth, DEC);
  Y = String(year, DEC);
  HR = String(hour, DEC);
  if (minute < 10)
  {
    MIN += '0';
  }
  MIN += String(minute, DEC);
  if (second < 10)
  {
    S += '0';
  }
  S += String(second, DEC);
  Text += Y + '-' + M + '-' + D + '-' + HR + ':' + MIN + ':' + S + ' '; //Desired Format 2017-12-01-16:34:23 which is //Y-M-D-HR:MIN:S
  //Text += ' ' + M + D + Y + '_' + HR + ':' + MIN+ ':' + S + ' '; //Displays MDY_HR:MIN
  delay(1000);
  return Text;
}



void finishUp() {
  pinMode(Coms2MCU, OUTPUT);
  digitalWrite(Coms2MCU, LOW);
  delay(80);
  digitalWrite(Coms2MCU, HIGH);
  delay(80);
  digitalWrite(Coms2MCU, LOW);
  pinMode(Coms2MCU, INPUT);
}


