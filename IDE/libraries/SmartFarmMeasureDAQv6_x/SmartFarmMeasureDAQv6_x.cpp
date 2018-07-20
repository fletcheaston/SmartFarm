// Originally created by Caleb Fink, updated and refactored by Fletcher Easton
// This library lets you read data from a SmartFarm DAQ, using either board 6.2 or 6.1

#include  "SmartFarmMeasureDAQv6_x.h"

// Define the pins and hardware on the boards
#define Coms2MCU 6 // Allows communication back to the MCU
#define WMEvenPin 4 // Defines the even pin for reading each watermark
#define WMOddPin 5 // Defines the odd pin for reading each watermark
#define muxAnalogRead 0 // Defines the MUX analog pin
#define readBatPin 6 // Defines the battery power pin
#define readSolPin 7 // Defines the solar power pin
#define DS3231_I2C_ADDRESS 0x68 // Defines the RTC address
#define ONE_WIRE_BUS 3 // ???
#define TEMPERATURE_PRECISION 12 // ???
#define DEC_PIN 2

const int select_pins[3] = {9, 20, 21}; // ???
DallasTemperature sensors(&oneWire); // ???
DeviceAddress temp_device_address; // ???
uint8_t stored_address[6][8]; // Stored the address of each temperature sensor
int temperature_sensor_count = 0; // Counts the number of temperature sensors
int decagon_sensor_count = 0; // Counts the number of decagon sensors
int max_decagon_sensors = 3; // Defines the maximum number of decagon sensors
const int chip_select = 10; // Defines the SD card chip info?
SDI12 smartSDI12(DEC_PIN); // ???
static byte address_register[8] = {0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000}; // ???

/******************************************************************************/
/* Low Level Functions */

// ???
SmartFarmMeasure::SmartFarmMeasure()
{
	// Does absolutely nothing but allows the script to compile?
}

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

// Converts character to decimal
byte SmartFarmMeasure::charToDec(char i)
{
	if ((i >= '0') && (i <= '9')) return i - '0'; //returns i as a byte
	if ((i >= 'a') && (i <= 'z')) return i - 'a' + 10; //decimal 35
	if ((i >= 'A') && (i <= 'Z')) return i - 'A' + 36; //changed from 37, 11/30/17 as this gave i as 62 instead of 61
}

// Converts decimal to character
char SmartFarmMeasure::decToChar(byte i)
{
	if ((i >= 0) && (i <= 9)) return i + '0';
	if ((i >= 10) && (i <= 36)) return i + 'a' - 10;
	if ((i >= 37) && (i <= 62)) return i + 'A' - 37;
}

// ???
boolean SmartFarmMeasure::checkActive(char i)
{
	String myCommand = "";
	myCommand = "";
	myCommand += (char) i;
	// sends basic 'acknowledge' command [address][!]
	myCommand += "!";

	for (int j = 0; j < 3; j++)
	{
		// goes through three rapid contact attempts
		smartSDI12.sendCommand(myCommand);
		if (smartSDI12.available() > 1) break;
		delay(30);
	}
	if (smartSDI12.available() > 2)
	{
		// if it hears anything it assumes the address is occupied
		smartSDI12.flush();
		return true;
	}
	else
	{   // otherwise it is vacant.
		smartSDI12.flush();
	}
	return false;
}

// Sets the bit in the proper location within the address_register to record that the sensor is active and that the address is taken
boolean SmartFarmMeasure::setTaken(byte i)
{
	boolean init_status = isTaken(i);
	i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61.
	byte j = i / 8;   // byte #
	byte k = i % 8;   // bit #
	address_register[j] |= (1 << k);
	return !init_status; // return false if already taken
}

// Unsets the bit in the proper location within the address_register.
boolean SmartFarmMeasure::setVacant(byte i)
{
	boolean init_status = isTaken(i);
	i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61.
	byte j = i / 8;   // byte #
	byte k = i % 8;   // bit #
	address_register[j] &= ~(1 << k);
	return init_status; // return false if already vacant
}

// Checks if the address has been taken by an active sensor
boolean SmartFarmMeasure::isTaken(byte i)
{
	i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61.
	byte j = i / 8;   // byte #
	byte k = i % 8;   // bit #
	return address_register[j] & (1 << k); // return bit status
}

// Sets the S0, S1, and S2 pins to select the given pin
void SmartFarmMeasure::selectMuxPin(byte pin)
{
	for (int i = 0; i < 3; i++)
	{
		delayMilliseconds(5);
		if (pin & (1 << i))
		{
			digitalWrite(select_pins[i], HIGH);
		}
		else
		{
			digitalWrite(select_pins[i], LOW);
		}
	}
}

// ???
String SmartFarmMeasure::getDevAddress(DeviceAddress device_address)
{
	String result = "";
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (device_address[i] < 16)
			result += '0';
		result += String(device_address[i], HEX);
	}
	return result;
}

// Reads the decagon sensor at the specified address
// This is a large function... I'd break it into smaller, more readable functions but it works and I'd rather not mess with it at this moment in time
String SmartFarmMeasure::takeDecMeasurement(char i)
{
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
	while ((millis() - timerStart) > (1000 * wait))
	{
		if (smartSDI12.available()) break;
		//sensor can interrupt us to let us know it is done early
	}

	// in this example we will only take the 'DO' measurement
	smartSDI12.flush();
	command = "";
	command += i;
	command += "D0!"; // SDI-12 command to get data [address][D][dataOption][!]
	smartSDI12.sendCommand(command);

	while (!smartSDI12.available() > 1); // wait for acknowlegement

	delay(300);

	String buffer =  "D" + String(i);
	smartSDI12.read();

	while (smartSDI12.available())
	{
		char c = smartSDI12.read();
		if (c == '+' || c == '-')
		{
			buffer += ' ';
			if (c == '-') buffer += '-';
		}
		else if (c == '\r' || c == '\n')
		{
			buffer += "";
		}
		else
		{
			buffer += String(c);
		}
		delay(100);
	}

	return buffer;
}

// Delays a specified number of milliseconds... using microseconds... Silly but whatever
void SmartFarmMeasure::delayMilliseconds(int x)
{
	for (int i = 0; i < x; i++)
	{
		delayMicroseconds(1000); // 1 millsec
	}
}

// Essentially resets the board after wireless uploading
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

/******************************************************************************/
/* RTC Functions */

// Sets the time for the RTC
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

// Reads the time from the RTC
void SmartFarmMeasure::readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
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

// Reads the time from the host computer and sets the time in the RTC
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

// Returns a String of the current time, according to the RTC
String SmartFarmMeasure::timeStamp()
{
	byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
	String M = ""; String D = ""; String Y = ""; String HR = ""; String MIN = ""; String S = ""; //strings for RTC
	String result;

	// retrieve data from DS3231
	readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
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

	return result;
}

/******************************************************************************/
/* Voltage Functions */

// Returns a String of the current voltage readings, from both the battery and the solar panel
String SmartFarmMeasure::readVolts()
{
	analogReference(EXTERNAL); // use AREF for reference voltage added 1/5/17
	String result;
	int BatVi = analogRead(readBatpin); //A6 Battery Voltage Pin
	int SolVi = analogRead(readSolpin); //A7 Solar Voltage pin
	float BatVF = (BatVi * 2.0 * 3.3) / 1023.0; //battery voltage the 2.0 and 3600 should be measured, though each board will vary slightly
	float SolVF = (SolVi * 2.0 * 3.3) / 1023.0; //solar voltage
	result = String(BatVF, 2) + " " + String(SolVF, 2);
	return result;
}

/******************************************************************************/
/* SD Card Functions */

// Sets up the SD card chip
void SmartFarmMeasure::setupSD()
{
	SD.begin(chipSelect);
}

// Writes a String to the SD card in a datalog file
void SmartFarmMeasure::write2SD(String dataString)
{
	File dataFile = SD.open("datalog.txt", FILE_WRITE);
	if (dataFile) // If the file is available, write to it:
		if (dataString == "newline")
		{
			dataFile.println();
			dataFile.close();
		}
		else
		{
			dataFile.print(dataString);
			dataFile.close();
		}
	else
	{
		// Some error has occured with the SD card write
	}
	dataString = "";
	delay(1200);
}

/******************************************************************************/
/* Setup Sensor Functions */

// An easy function to call to setup each main part of the DAQ
void SmartFarmMeasure::setupAll()
{
	Serial.begin(57600);
	setupSD();
	delay(1000);
	setupWM();
	delay(1000);
	setupTemps();
	delay(1000);
        setupDecSensors();
        delay(1000);
}

// Sets up the watermark ports
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

// Sets up the temperature ports
void SmartFarmMeasure::setupTemps()
{
	// Counts devices, checks parasite power mode, calls the store address function, sets the precision, & sorts the devices
	uint8_t temp[8];
	boolean swap = false;
	unsigned long swapnewn;
	unsigned long swapn = 8;

	// Start up the library
	sensors.begin();
	// Grab a count of devices on the wire
	numberOfDevices = sensors.getDeviceCount();

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
			// Set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
			sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

    		}
  	}
	// Loop through devices and sort them
	for (uint8_t col = 0; col < 8; col++)//temp index
	{
		// Swapped is true, then don't do anymore
		unsigned long newn;
		unsigned long n = numberOfDevices;

		do
		{
			newn = 1;
			for (uint8_t sensorIndex = 1; sensorIndex < numberOfDevices; sensorIndex++)//temp index
			{
        			if (storedAddress[sensorIndex - 1][col] > storedAddress[sensorIndex][col])
        			{
          				for (uint8_t i = 0; i < 8; i++)//swap sensors
          				{
						// Bubble sort algorithm
						// Credit By: dndubins
						// https://playground.arduino.cc/Main/QuickStats
						temp[i] = storedAddress[sensorIndex][i];
						storedAddress[sensorIndex][i] = storedAddress[sensorIndex - 1][i];
						storedAddress[sensorIndex - 1][i] = temp[i];
					} // End swap

					swap = true;
					newn = sensorIndex;
				}
			}

			n = newn;
		}
		while (n > 1);
		{
	    		if (swap == true)
	    		{
				col = 9;
	    		}
		}
  	}
}

// Sets up the decagon ports
void SmartFarmMeasure::setupDecSensors()
{
	smartSDI12.begin();
	delay(500); // allow things to settle

	for (byte i = '0'; i <= '9'; i++) if (checkActive(i)) setTaken(i);  // scan address space 0-9

	for (byte i = 'a'; i <= 'z'; i++) if (checkActive(i)) setTaken(i); // scan address space a-z

	for (byte i = 'A'; i <= 'Z'; i++) if (checkActive(i)) setTaken(i); // scan address space A-Z

	found = 0;

	for (byte i = 0; i < 62; i++)
	{
		if (isTaken(i))
		{
			found++;
		}
	}
}

/******************************************************************************/
/* Read Sensor Functions */

// Returns a String of the watermark port readings
String SmartFarmMeasure::readWM(int count)
{
	byte WC = 0B00000000; // Watermark connection check
	int WMPin1 = WMEvenPin;
	int WMPin2 = WMOddPin;
	String result;

	for (byte i = 0; i < count; i++) // Go through ports to read data
	{
		String Rstring = "";
		float RArray[5];
		float Rs;
		float R; // Sensor resistance
		float v; // Volt measurements
		selectMuxPin(i);
		// Take measurements 5 times
		for (int j = 0; j < 5; j++)
		{
			digitalWrite(WMPin1, LOW); // Sets the pin on
			digitalWrite(WMPin2, HIGH); // Sets the pin off
			delayMilliseconds(5); // 100 Hz*/

			digitalWrite(WMPin1, HIGH); // Sets the pin on
			digitalWrite(WMPin2, LOW); // Sets the pin off
			delayMilliseconds(5); // 100 Hz
			int a = analogRead(muxAnalogRead);//

			digitalWrite(WMPin1, LOW); // Shut down the watermark power
			digitalWrite(WMPin2, LOW);

			v = 3.3 * a / 1023.0; // Take another reading
			R = 10000.0 * v / (3.3 - v); // Voltage divider rule to find resistance value
			if (R > 0.0)
      			{
        			RArray[j] = R;
      			}
      			else
			{
        			RArray[j] = -1;
      			}
    		}

		// Sort the resistance array and calculate the average of the middle three readings
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
			if(Rstring == " INF")
			{
				Rstring = "INF";
			}
		}
		else
		{
			Rstring = F("NA");
		}

		result += Rstring;
		result += " ";
	}

	// Finish each port
	for (int i = 0; i < 3; i++)
	{
		pinMode(selectPins[i], OUTPUT);
		digitalWrite(selectPins[i], LOW);
		delayMilliseconds(5);
  	}

  	return result;
}

// Returns a String of the temperature port readings
String SmartFarmMeasure::readTemps(int count)
{
	String result;
	uint8_t *address;
	char fbuff[8];
	float data;

	sensors.requestTemperatures(); // Send the command to get temperatures, gets senors ready for measurement
	int tempPosition[] = {0,1,2,3,4,5,6,7,8,9};
	// If you're reading more than 10 temperature sensors... don't...

	for (int i = 0;i < count;i++)
	{
		address  = storedAddress[tempPosition[i]];
		data = sensors.getTempC(address);
		dtostrf(data, 5, 2, fbuff);
		result += fbuff;
		result += " ";
	}

	return result;
}

// Returns a String of the decagon port readings
String SmartFarmMeasure::readDecSensors()
{
	String result = "";

	// Scan  address space 0-9
	for (char i = '0'; i <= '9'; i++) if (isTaken(i))
	{
		result += takeDecMeasurement(i);
		smartSDI12.flush();
	}

	// Scan address space a-z
	for (char i = 'a'; i <= 'z'; i++) if (isTaken(i))
	{
		result += takeDecMeasurement(i);
		smartSDI12.flush();
	}

	// Scan address space A-Z
	for (char i = 'A'; i <= 'Z'; i++) if (isTaken(i))
	{
		result += takeDecMeasurement(i);
		smartSDI12.flush();
	}

	return result;
}
