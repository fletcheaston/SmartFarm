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

void SmartFarmMeasure::setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
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
	String result = "";
	int BatVi = analogRead(readBatpin); //A6 Battery Voltage Pin
	int SolVi = analogRead(readSolpin); //A7 Solar Voltage pin
	float BatVF = (BatVi * 2.0 * 3.3) / 1023.0; //battery voltage the 2.0 and 3600 should be measured, though each board will vary slightly
	float SolVF = (SolVi * 2.0 * 3.3) / 1023.0; //solar voltage
	result += "Battery-Voltage: " + String(BatVF, 2);
	result += " Solar-Voltage: " + String(SolVF, 2);
	return result;
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
}

void SmartFarmMeasure::setupSD()
{
	SD.begin(chipSelect);
}

void SmartFarmMeasure::write2SD(String dataString)
{
	File dataFile = SD.open("datalog.txt", FILE_WRITE);
	if (dataFile) // if the file is available, write to it:
		if (dataString == "newline")
		{
			dataFile.println();
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
		//Serial.println("Error: SD Card not found")
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
	int tempPosition[] = {0,1,2,3,4,5,6,7,8,9};
	// If you're reading more than 10 temperature sensors... don't...

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
void SmartFarmMeasure::setupTemps()
{
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

/** setupWM
   * Sets up resistive port for watermark sensors.
   **/
void SmartFarmMeasure::setupWM()
{
	//Set up the power port
	pinMode(WMEvenPin, OUTPUT);
	pinMode(WMOddPin, OUTPUT);
	//Set up the select pins as outputs
	for (int i = 0; i < 3; i++)
	{
		pinMode(selectPins[i], OUTPUT);
		digitalWrite(selectPins[i], LOW); //initialize to first mux pin Y0, the first WM pin
		delayMilliseconds(5);
	}
}

//call this function at the beginning of the DAQ program to finish wireless uploading
void SmartFarmMeasure::finishUp()
{
	pinMode(Coms2MCU, OUTPUT);
	digitalWrite(Coms2MCU, LOW);
	delay(80);
	digitalWrite(Coms2MCU, HIGH);
	delay(80);
	digitalWrite(Coms2MCU, LOW);
	pinMode(Coms2MCU, INPUT);
}

// The selectMuxPin function sets the S0, S1, and S2 pins to select the given pin
void SmartFarmMeasure::selectMuxPin(byte pin)
{
	for (int i = 0; i < 3; i++)
	{
		delayMilliseconds(5);
		if (pin & (1 << i))
		{
			digitalWrite(selectPins[i], HIGH);
		}
		else
		{
			digitalWrite(selectPins[i], LOW);
		}
	}
}

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
