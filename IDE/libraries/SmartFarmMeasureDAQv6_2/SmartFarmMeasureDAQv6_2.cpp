#include "SmartFarmMeasureDAQv6_2.h"
//2/10/18
//Adapted for circuit DAQ v6.2
//11/29/17
//Included String boardID in read functions for data parsing from network
//End each serial print data segment with a newline for easy parsing on the Pi
//INCLUDED MUX FOR WATERMARK PROGRAM 12/29/17

//BY: Caleb Fink
//Updated by: Fletcher Easton

//Logic vs hardware

/*
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
*/



//Defined pins
//#define XBEE_RADIO_BAUD 57600 not sure how to call this defined constant variable in the ino file from this library. So I placed it insinde the ino file for now.
#define Coms2MCU 6 //used in finishUp function and communicating back to the mcu. Can use an FSM on this pin in the mcu firmware...
//to communicate different stages in the DAQ during the measurement cycle, like when the measurement is done, the DAQ could wiggle or indicate that it is done and also when to enter program mode as well as if a program is ready.
//As in, RPi or base station can tell the DAQ board "Hey I've got a new program for you, Finish the measurement data then I'll send it over to you". The DAQ would respond with "ok, here's measurement data, I'm going into programming mode".
//In this way the wireless programming mode could be shutoff and only available when the base station requests it. That's if the DAQ can receive requests, which it should be able to.
#define WMEvenPin 4 //used in setupWM function
#define WMOddPin 5 //used in setupWM function
#define muxAnalogRead 0 //used in setupMUXAnalog & readMUXAnalog function
#define readBatpin 6 //used in readVolts
#define readSolpin 7 //used in readVolts
const int selectPins[3] = {9, 20, 21}; // S-pins to DAQ pins: S0~9, S1~20, S2~21
#define DS3231_I2C_ADDRESS 0x68 //Defined RTC Address
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 12
#define MAX_SENSORS 6 //used in read temp per side
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// DeviceAddress therms[MAX_SENSORS]; //device address array
DeviceAddress tempDeviceAddress; //used in read temps
uint8_t storedAddress[6][8]; // used in read temps
int numberOfDevices = 0; // Number of temperature devices found
int found = 0;
int MaxDecSens = 3;
//Needed variables for Decagon sensors
#define DEC_PIN 2
SDI12 smartSDI12(DEC_PIN);
static byte addressRegister[8] = {
  0B00000000,
  0B00000000,
  0B00000000,
  0B00000000,
  0B00000000,
  0B00000000,
  0B00000000,
  0B00000000
};

const int chipSelect = 10; //sdcard

// Convert normal decimal numbers to binary coded decimal
byte SmartFarmMeasure::decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
// Convert binary coded decimal to normal decimal numbers
byte SmartFarmMeasure::bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void SmartFarmMeasure::setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year) {
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



void SmartFarmMeasure::readDS3231time(byte *second,//RTC function
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



//this function sets the RTC time from the compile time
void SmartFarmMeasure::setRTCToComputerTime(char myDATEString[], char myTIMEString[])
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
	switch (myDATEString[0])
	{
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

String SmartFarmMeasure::readVolts() //Battery & Solar voltage reporting
{
	analogReference(EXTERNAL); // use AREF for reference voltage added 1/5/17
	String Text = "";
	int BatVi = analogRead(readBatpin); //A6 Battery Voltage Pin
	int SolVi = analogRead(readSolpin); //A7 Solar Voltage pin
	float BatVF = (BatVi * 2.0 * 3.3) / 1023.0; //battery voltage the 2.0 and 3600 should be measured, though each board will vary slightly
	float SolVF = (SolVi * 2.0 * 3.3) / 1023.0; //solar voltage
	Text += "Battery Voltage: " + String(BatVF, 2);
	Text += " Solar Voltage: " + String(SolVF, 2);
	return Text;
}

String SmartFarmMeasure::timeStamp() //RTC function
{
	byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
	String M = ""; String D = ""; String Y = ""; String HR = ""; String MIN = ""; String S = ""; //strings for RTC
	String result;

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
	result = Y + "/" + M + "/" + D + "/" + HR + ":" + MIN + ":" + S;
	//result += Y + '-' + M + '-' + D + '-' + HR + ':' + MIN + ':' + S + ' '; //Desired Format
	return result;
}

void SmartFarmMeasure::setupAll()
{
	Serial.begin(57600);
	setupSD();
	delay(1000);
	setupWM();
	delay(1000);
	setupTemps();
	delay(1000);
	setupDecSensors("");
	delay(1000);
}




void SmartFarmMeasure::runAll(String boardID)
{
	//readWM(boardID);
	delay(1000);
	//readTemps(boardID);
	delay(1000);
	readDecSensors();
	delay(1000);
	printUpload(boardID);
	Serial.flush();
	finishUp();
	return;
}




void SmartFarmMeasure::setupSD()
{
	SD.begin(chipSelect);
}




void SmartFarmMeasure::write2SD(String dataString) {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) // if the file is available, write to it:
    if (dataString == "newline")
    { dataFile.println();
      dataFile.close();
      //Serial.println("***SD Card written.***\n");
    }
	else
	{
      dataFile.print(dataString);
      dataFile.close();
      //Serial.println("***SD Card written.***\n");
    }
  else // if the file isn't open, pop up an error:
  {
    //Serial.println("XXXSD Card not found.XXX\n");
  }
  dataString = "";
  delay(1200);
}




  /** get_temp
   * Stores temperature data.
   * inputs:
   *    data: (float*) storage for temperature data (at least numtempsens big)
   *    numtempsens: (int) number of temperature sensors
   *    tempPos1: (int) identifying value for sensor 1 (0-127 valid sensors, -1 for Null)
   *    tempPos2: (int) identifying value for sensor 2 (0-127 valid sensors, -1 for Null)
   *    tempPos3: (int) identifying value for sensor 3 (0-127 valid sensors, -1 for Null)
   **/

void SmartFarmMeasure::get_temp(float* data, int numtempsens, int tempPos1, int tempPos2, int tempPos3)
{
  sensors.requestTemperatures(); // Send the command to get temperatures, gets senors ready for measurement
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  int tempPosition[] = {tempPos1 - 1, tempPos2 - 1, tempPos3 - 1};
  uint8_t *address;
  // Loop through each device, store temperature data
  for (int i = 0; i < numtempsens; i++)
  {
    //copy each storedaddress into a temporary device address type
    address  = storedAddress[tempPosition[i]];
    data[i] = sensors.getTempC(address);
  }
}

/** build_data_string...
   * Builds a data string.
   * The format looks like this "27.00 27.00 27.00" or "NA NA NA" or a combination
   * inputs:
   *    data: (float*) pointer to temperature float data
   *    numtempsens: (int) number of temperature sensors
   *    results: (String) node and sensor identifier
   * return:
   *     String: complete string of data and/or placeholders
   *
String SmartFarmMeasure::build_data_string(float* data, int numtempsens, String results)
{
  String NA = F("NA");
  char fbuff[8]; //float to char buffer
  for (int i = 0; i <= 2; i++)
  {
    if (data[i] < 0.0)
    {
      results += NA;
    }
    else
    {
      dtostrf(data[i], 5, 2, fbuff); //builds a string from a floating point value
      //  dtostrf(FLOAT,WIDTH,PRECISION,BUFFER)
      //WIDTH = PRECISION+2
      //PRECISION = the number of characters after the decimal point
      //BUFFER = storage of the char string
      //Serial.print(" ");
      //  Serial.println(fbuff);
      //results += "12.34 ";
      results += fbuff;
    }
    results += ' ';
  }
  return results;
}*/



// This returns the temp sensor data as a string
//new for v6.1 and v6.2, east and west sensors
/** readTemps...
   * Constructs a ID string for Temperature sensor and node identification over network.
   * The format looks like this "N3 ST123 "
   * inputs:
   *    boardID: (String) identifies which node in a network
   *    tempPos1: (int) identifying value for sensor 1 (0-127 valid sensors, -1 for Null)
   *    tempPos2: (int) identifying value for sensor 2 (0-127 valid sensors, -1 for Null)
   *    tempPos3: (int) identifying value for sensor 3 (0-127 valid sensors, -1 for Null)
   * return:
   *     String: formated node and sensor ID
   **/

String SmartFarmMeasure::readTemps(int count)
{
	String results;
	uint8_t *address;
	char fbuff[8];
	float data;

	if(count <= 0)
	{
		results = F("No Temperatures");
		return results;
	}
	else if(count == 1)
	{
		results = F("Temperature:");
	}
	else
	{
		results = F("Temperatures:");
	}

	sensors.requestTemperatures(); // Send the command to get temperatures, gets senors ready for measurement
	int tempPosition[] = {0,1};

	for (int i = 0;i < count;i++)
	{
		address  = storedAddress[tempPosition[i]];
		data = sensors.getTempC(address);
		dtostrf(data, 5, 2, fbuff);
		results += " ";
		results += fbuff;
	}

	return results;
}

//setup temps should address the sensors, sort them and store them in an array to access in read temps west and east functions
void SmartFarmMeasure::setupTemps() {
  //this function: counts devices, checks parasite power mode, calls the store address function, sets the precision, & sorts the devices
  //Test algorithm with sleep
  //Test algorithm with power off/lose power (unplug battery and plug back in)
  uint8_t temp[8];
  boolean swap = false;
  unsigned long swapnewn;
  unsigned long swapn = 8;

  // Start up the library
  sensors.begin();
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  // locate devices on the bus
  /*   Serial.print("Locating devices...");
    Serial.print("Found ");
    Serial.print(numberOfDevices, DEC);
    Serial.println(" devices."); */

  // Loop through each device and print out addresses
  for (int i = 0; i < numberOfDevices; i++)
  {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i))
    {
      for (uint8_t x = 0; x < 8; x++)
      {
        storedAddress[i][x] = tempDeviceAddress[x];
      }
      //print out the addresses and store them for sorting later
      /*       Serial.print("Storing Device ");
            Serial.print(i, DEC);
            Serial.print(" as address: ");
            for (uint8_t q = 0; q < 8; q++)
            {
              if (storedAddress[i][q] < 16) Serial.print("0");
              Serial.print(storedAddress[i][q], HEX);
            }
            Serial.println(); */

      // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

    } /* else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    } */
  }
  //loop through devices and sort them
  for (uint8_t col = 0; col < 8; col++)//temp index
  {
    //swapped is true, then don't do anymore
    unsigned long newn;
    unsigned long n = numberOfDevices;
    do {
      newn = 1;
      for (uint8_t sensorIndex = 1; sensorIndex < numberOfDevices; sensorIndex++)//temp index
      {
        if (storedAddress[sensorIndex - 1][col] > storedAddress[sensorIndex][col])
        {
          for (uint8_t i = 0; i < 8; i++)//swap sensors
          {
            //bubble sort algorithm
            //credit By: dndubins
            //https://playground.arduino.cc/Main/QuickStats
            temp[i] = storedAddress[sensorIndex][i];
            storedAddress[sensorIndex][i] = storedAddress[sensorIndex - 1][i];
            storedAddress[sensorIndex - 1][i] = temp[i];
          }//end swap
          //swapnewn = row;
          swap = true;
          newn = sensorIndex;
        }//end if
      }//end for
      n = newn;
    } while (n > 1);
    if ( swap == true)
    {
      col = 9;
    }
  }
  /*Serial.println("---Devices sorted least to greatest address---");

  // Loop through each device, printing the sorted devices
    for (int sensorIndex = 0; sensorIndex < numberOfDevices; sensorIndex++)
    {
      Serial.print("Sorted Device ");
      Serial.print(sensorIndex, DEC);
      Serial.print(" as address: ");
      for (uint8_t i = 0; i < 8; i++)
      {
        if (storedAddress[sensorIndex][i] < 16) Serial.print("0");
        Serial.print(storedAddress[sensorIndex][i], HEX);
      }
      Serial.println();
    } */
}




String SmartFarmMeasure::getDevAddress(DeviceAddress deviceAddress)
{
  String result = "";
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16)
      result += '0';
    result += String(deviceAddress[i], HEX);
  }
  return result;
}




String SmartFarmMeasure::setupDecSensors(String boardID) {
  String Text = boardID +F(" DMECT");
  smartSDI12.begin();
  delay(500); // allow things to settle

  //Scanning Decagon 5TE addresses
  for (byte i = '0'; i <= '9'; i++) if (checkActive(i)) setTaken(i);  // scan address space 0-9

  for (byte i = 'a'; i <= 'z'; i++) if (checkActive(i)) setTaken(i); // scan address space a-z

  for (byte i = 'A'; i <= 'Z'; i++) if (checkActive(i)) setTaken(i); // scan address space A-Z

  found = 0;

  for (byte i = 0; i < 62; i++) {
    if (isTaken(i)) {
      found++;
    }
  }

  if (!found) {
    Text += F(" No DECAGON 5TE sensors found.");
  } // stop here
	else{
		  Text += F(" Found ");
  Text += String(found, DEC);
  Text += F(" Decagon 5TE sensors.");
	}
  return Text;
}




String SmartFarmMeasure::readDecSensors()
{
	String result = F("Decagon:");
	// result ="Reading Decagon 5TE Sensors... \n";
	// result += "VWC(Dielectric) EC(dS/m) Temperature(Deg C)\n";

	for (char i = '0'; i <= '9'; i++) if (isTaken(i))
	{

		//printInfo(i);
		//Prints "#13DECAGON 5TE   365." which is "Sensor ID, Type, and Version."
		result += takeDecMeasurement(i);//This prints " 1.01 0.00 24.7"
		smartSDI12.flush();
		//Serial.print(result);
	}

	// scan address space a-z
	for (char i = 'a'; i <= 'z'; i++) if (isTaken(i))
	{
		// result += " D" + String(i); //should be the id of the dec sensor  printing "# "
		// write2SD(result); //write
		// Serial.print(result); //print
		// result = "";
		//printInfo(i);        //Prints "#13DECAGON 5TE   365." which is "Sensor ID, Type, and Version."
		result += takeDecMeasurement(i);        //This prints " 1.01 0.00 24.7"
		smartSDI12.flush();
		//Serial.print(result);
	}

	// scan address space A-Z
	for (char i = 'A'; i <= 'Z'; i++) if (isTaken(i))
	{
		// result += " D" + String(i); //should be the id of the dec sensor  printing "# "
		// write2SD(result); //write
		// Serial.print(result); //print
		// result = "";
		//printInfo(i);        //Prints "#13DECAGON 5TE   365." which is "Sensor ID, Type, and Version."
		result += takeDecMeasurement(i);        //This prints " 1.01 0.00 24.7"
		smartSDI12.flush();
		// Serial.print(result);
	}

	return result;
}


String SmartFarmMeasure::takeDecMeasurement(char i) {
  String command = "";
  //String result = "";
  command += i; //address of sensor from for loop above
  command += "M!"; // SDI-12 measurement command format  [address]['M'][!]
  smartSDI12.sendCommand(command);
  while (!smartSDI12.available() > 5); // wait for acknowlegement with format [address][ttt (3 char, seconds)][number of measurments available, 0-9]
  delay(100);

  smartSDI12.read(); //consume address

  // find out how long we have to wait (in seconds).
  int wait = 0;
  wait += 100 * smartSDI12.read() - '0';
  wait += 10 * smartSDI12.read() - '0';
  wait += 1 * smartSDI12.read() - '0';

  smartSDI12.read(); // ignore # measurements, for this simple example
  smartSDI12.read(); // ignore carriage return
  smartSDI12.read(); // ignore line feed

  long timerStart = millis();
  while ((millis() - timerStart) > (1000 * wait)) {
    if (smartSDI12.available()) break;               //sensor can interrupt us to let us know it is done early
  }

  // in this example we will only take the 'DO' measurement
  smartSDI12.flush();
  command = "";
  command += i;
  command += "D0!"; // SDI-12 command to get data [address][D][dataOption][!]
  smartSDI12.sendCommand(command);
  while (!smartSDI12.available() > 1); // wait for acknowlegement
  delay(300); // let the data transfer
  String buffer = " D" + String(i); //should be the id of the dec sensor  printing "# "
  smartSDI12.read(); // consume address
  while (smartSDI12.available()) {
    char c = smartSDI12.read();
    if (c == '+' || c == '-') {
      buffer += ' ';  // this may need to be changed back
      if (c == '-') buffer += '-';
    }
    else if (c == '\r' || c == '\n') {
      buffer += "";
      // break;
    }
    else {
      buffer += String(c);
      //if (c == '\n') buffer += "NEWLINE CHARACTER";
    }
    delay(100);
  }
  return buffer;

}

// this checks for activity at a particular address
// expects a char, '0'-'9', 'a'-'z', or 'A'-'Z'
boolean SmartFarmMeasure::checkActive(char i) {

  String myCommand = "";
  myCommand = "";
  myCommand += (char) i;                 // sends basic 'acknowledge' command [address][!]
  myCommand += "!";

  for (int j = 0; j < 3; j++) {          // goes through three rapid contact attempts
    smartSDI12.sendCommand(myCommand);
    if (smartSDI12.available() > 1) break;
    delay(30);
  }
  if (smartSDI12.available() > 2) {   // if it hears anything it assumes the address is occupied
    smartSDI12.flush();
    return true;
  }
  else {   // otherwise it is vacant.
    smartSDI12.flush();
  }
  return false;
}




// this sets the bit in the proper location within the addressRegister
// to record that the sensor is active and the address is taken.
boolean SmartFarmMeasure::setTaken(byte i) {
  boolean initStatus = isTaken(i);
  i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61.
  byte j = i / 8;   // byte #
  byte k = i % 8;   // bit #
  addressRegister[j] |= (1 << k);
  return !initStatus; // return false if already taken
}




// THIS METHOD IS UNUSED IN THIS EXAMPLE, BUT IT MAY BE HELPFUL.
// this unsets the bit in the proper location within the addressRegister
// to record that the sensor is active and the address is taken.
boolean SmartFarmMeasure::setVacant(byte i) {
  boolean initStatus = isTaken(i);
  i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61.
  byte j = i / 8;   // byte #
  byte k = i % 8;   // bit #
  addressRegister[j] &= ~(1 << k);
  return initStatus; // return false if already vacant
}




// this quickly checks if the address has already been taken by an active sensor
boolean SmartFarmMeasure::isTaken(byte i) {
  //Serial.write(i);
  //Serial.println();
  i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61.
  //Serial.write(i);
  //Serial.println();
  byte j = i / 8;   // byte #
  byte k = i % 8;   // bit #
  return addressRegister[j] & (1 << k); // return bit status
}




// gets identification information from a sensor, and prints it to the serial port
////This writes " --, i13DECAGON 5TE   365" to SD card
// expects a character between '0'-'9', 'a'-'z', or 'A'-'Z'.
char SmartFarmMeasure::printInfo(char i) {
  int j;
  String command = "";
  String result = "";
  command += (char) i;
  command += "I!";
  for (j = 0; j < 1; j++) {
    smartSDI12.sendCommand(command);
    delay(30);
    if (smartSDI12.available() > 1) break;
    if (smartSDI12.available()) smartSDI12.read();
  }

  while (smartSDI12.available()) {
    char c = smartSDI12.read();
    if ((c != '\n') && (c != '\r')) {
      //Serial.write(c);  //Prints "#13DECAGON 5TE   365" which is "Sensor ID, Type, and Version"
      result = String(c);
      if (c == '0') {//**************************changed from '1' to '0'
        break; //break after the id
      }
    }
    delay(5);
  }
}




// converts allowable address characters '0'-'9', 'a'-'z', 'A'-'Z',
// to a decimal number between 0 and 61 (inclusive) to cover the 62 possible addresses
byte SmartFarmMeasure::charToDec(char i) {
  if ((i >= '0') && (i <= '9')) return i - '0'; //returns i as a byte
  if ((i >= 'a') && (i <= 'z')) return i - 'a' + 10; //decimal 35
  if ((i >= 'A') && (i <= 'Z')) return i - 'A' + 36; //changed from 37, 11/30/17 as this gave i as 62 instead of 61
}




// THIS METHOD IS UNUSED IN THIS EXAMPLE, BUT IT MAY BE HELPFUL.
// maps a decimal number between 0 and 61 (inclusive) to
// allowable address characters '0'-'9', 'a'-'z', 'A'-'Z',
char SmartFarmMeasure::decToChar(byte i) {
  if ((i >= 0) && (i <= 9)) return i + '0';
  if ((i >= 10) && (i <= 36)) return i + 'a' - 10;
  if ((i >= 37) && (i <= 62)) return i + 'A' - 37;
}




/** setupWM
   * Sets up resistive port for watermark sensors.
   **/
void SmartFarmMeasure::setupWM() {
  //Set up the power port
  pinMode(WMEvenPin, OUTPUT);
  pinMode(WMOddPin, OUTPUT);
  //Set up the select pins as outputs
  for (int i = 0; i < 3; i++) {
    pinMode(selectPins[i], OUTPUT);
    digitalWrite(selectPins[i], LOW); //initialize to first mux pin Y0, the first WM pin
    delayMilliseconds(5);
  }
}




void SmartFarmMeasure::test2_setupWM() {
  //Set up the power port
  pinMode(WMEvenPin, OUTPUT);
  pinMode(WMOddPin, OUTPUT);
  analogReference(EXTERNAL); // use AREF for reference voltage used for test2_readWM function
}




//call this function at the beginning of the DAQ program to finish wireless uploading
void SmartFarmMeasure::finishUp() {
  pinMode(Coms2MCU, OUTPUT);
  digitalWrite(Coms2MCU, LOW);
  delay(80);
  digitalWrite(Coms2MCU, HIGH);
  delay(80);
  digitalWrite(Coms2MCU, LOW);
  pinMode(Coms2MCU, INPUT);
}




// The selectMuxPin function sets the S0, S1, and S2 pins to select the given pin
void SmartFarmMeasure::selectMuxPin(byte pin) {
  for (int i = 0; i < 3; i++)
  {
    delayMilliseconds(5);
    if (pin & (1 << i))
      digitalWrite(selectPins[i], HIGH);
    else
      digitalWrite(selectPins[i], LOW);
  }
}




/**
   * readWM
   * Reads in watermark data from watermark sensors.
   * inputs:
   *    boardID: (String) identifies which node in a network
   *    WMpos1: (int) identifying value for sensor 1
   *    WMpos2: (int) identifying value for sensor 2
   *    WMpos3: (int) identifying value for sensor 3
   * return:
   *     String: formated node, data header and sensor data
   **/
String SmartFarmMeasure::readWM(int count)
{
	byte WC = 0B00000000; //watermark connection check
	int WMPin1 = WMEvenPin;
	int WMPin2 = WMOddPin;
	String WMdata;

	if(count <= 0)
	{
		WMdata = F("No Watermarks");
		return WMdata;
	}
	else if(count == 1)
	{
		WMdata = F("Watermark:");
	}
	else
	{
		WMdata = F("Watermarks:");
	}

	for (byte i = 0; i < count; i++) // Go through ports 1-6 to read data
	{
		String Rstring = " ";
		float RArray[5];
		float Rs;
		float R; //sensor resistance
		float v; //volt measurements
		selectMuxPin(i);
		//take measurements 5 times
		for (int j = 0; j < 5; j++)
		{
			digitalWrite(WMPin1, LOW); // sets the pin on
			digitalWrite(WMPin2, HIGH); // sets the pin off
			delayMilliseconds(5); // 100 Hz*/

			digitalWrite(WMPin1, HIGH); // sets the pin on
			digitalWrite(WMPin2, LOW); // sets the pin off
			delayMilliseconds(5); // 100 Hz
			int a = analogRead(muxAnalogRead);//

			digitalWrite(WMPin1, LOW); // Shut down the watermark power
			digitalWrite(WMPin2, LOW);

			//////// do the math after the watermark is powered off
			v = 3.3 * a / 1023.0; //take another reading
			R = 10000.0 * v / (3.3 - v); //voltage divider rule to find resistance value
			if (R > 0.0)  //watermark200ss sensors should range between...
      			{
        			RArray[j] = R;
      			}
      			else
			{
        			RArray[j] = -1;
      			}
    		}

		// sort the resistance array and calculate the average
    		for (int k = 0; k < 4; k++)
		{
			for (int o = 0; o < (5 - (k + 1)); o++)
			{
				if (RArray[o] > RArray[o + 1])
				{
					float temp = RArray[o];
					RArray[o] = RArray[o + 1];
					RArray[o + 1] = temp;
				}
			}
		}
		Rs = (RArray[1] + RArray[2] + RArray[3]) / 3;

		if (Rs > 0.0)
		{
			Rstring = String(Rs);
		}
		else
		{
			Rstring = F("NA");
		}
		WMdata = WMdata + ' ' + Rstring; // resistor reading for sensor i

	}

	//finish each port
	for (int i = 0; i < 3; i++)
	{
		pinMode(selectPins[i], OUTPUT);
		digitalWrite(selectPins[i], LOW);
		delayMilliseconds(5);
  	}

  	return WMdata;
}

void SmartFarmMeasure::delayMilliseconds(int x)
{
	for (int i = 0; i < x; i++)
	{
		delayMicroseconds(1000);// 1 millsec
	}
}
