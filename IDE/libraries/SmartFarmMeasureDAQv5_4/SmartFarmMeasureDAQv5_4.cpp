#include "SmartFarmMeasure.h"
//ALTERED 9/20/17
//BY: Caleb Fink

//Needed variables for temp sensors
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 9
#define MAX_SENSORS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress therms[MAX_SENSORS];
static int numsens = 0; //static is only for one function
int found = 0; //
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

const int chipSelect = 10;

void SmartFarmMeasure::printUpload() {
  Serial.println("Upload ");
  write2SD("Upload ");
}


void SmartFarmMeasure::setupAll() {
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

void SmartFarmMeasure::runAll() {
  readWM();
  delay(1000);
  readTemps();
  delay(1000);
  readDecSensors(MaxDecSens);
  delay(1000);
  printUpload();
  Serial.flush();
  finishUp();
  return;
}

SmartFarmMeasure::SmartFarmMeasure() {

}

void SmartFarmMeasure::setupSD() {
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
    else {
      dataFile.print(dataString);
      dataFile.close();
      //Serial.println("***SD Card written.***\n");
    }
  // else // if the file isn't open, pop up aan error:
  // {
  //Serial.println("XXXSD Card not found.XXX\n");
  // }
  dataString = "";
  //  Serial.flush();
  delay(1200);
}

void SmartFarmMeasure::readTemps() {
  String NA = "NA";
  String results = "";
  int i = 0;
  int placeholder = MAX_SENSORS - numsens; // 4-count, expected value = 4-4,4-3,4-2,4-1,4-0 = 0,1,2,3,4 NAs to print
  sensors.requestTemperatures();
  // print the device information
  for (i = 0; i < numsens; i++) { //sensors exist
    //results += printData(therms[i]);
    printData(therms[i]);
  }
  for (i = 0; i < placeholder; i++) {
    results += NA + ' '; //should print NA NA NA NA if no sensors found. should print NA NA NA
  }
  Serial.print(results);
  //delay(1000);
  write2SD(results);
}

void SmartFarmMeasure::printData(DeviceAddress deviceAddress)
{
  String result = "";
  float tempC;
  //float tempF;
  //NOTE 8/12/17: Discovered that the float value hogs a lot of space when writing to the sd card, and so never really writes to it effectively.
  //results += getDevAddress(deviceAddress);
  // results += " ";
  tempC = sensors.getTempC(deviceAddress);
  //results += "Temp C: ";
  result = String(tempC, 2) + ' ';
  /*   results += " Temp F: ";
    tempF = DallasTemperature::toFahrenheit(tempC);
    results += String(tempF); */
  Serial.print(result);
  write2SD(result);
}

void SmartFarmMeasure::setupTemps() {
  // String initstr = "Finding temperature sensors: Found ";
  // String toprint = "";
  sensors.begin();
  numsens = sensors.getDeviceCount();
  // initstr += String(numsens, DEC);
  // initstr += " devices.";
  // Serial.println(initstr);

  for (int i = 0; i < numsens; i++)
    if (!sensors.getAddress(therms[i], i)) Serial.println("Unable to find address for Device " + i);

  for (int i = 0; i < numsens; i++) {
    /*     toprint = "Device Address ";
      toprint += String(i, DEC);
      toprint += ": ";
      toprint += getDevAddress(therms[i]); */
    sensors.setResolution(therms[i], TEMPERATURE_PRECISION);
    /*     toprint += '\n';
      toprint += "Device Resolution ";
      toprint += String(i, DEC);
      toprint += ": ";
      toprint += String(sensors.getResolution(therms[i]), DEC);
      toprint += "\n"; */
    //Serial.println(toprint);
  }
  //Serial.println(toprint);
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

void SmartFarmMeasure::setupDecSensors() {
  smartSDI12.begin();
  delay(500); // allow things to settle

  //Serial.println("Scanning Decagon 5TE addresses.");

  for (byte i = '0'; i <= '9'; i++) if (checkActive(i)) setTaken(i); // scan address space 0-9

  for (byte i = 'a'; i <= 'z'; i++) if (checkActive(i)) setTaken(i); // scan address space a-z

  for (byte i = 'A'; i <= 'Z'; i++) if (checkActive(i)) setTaken(i); // scan address space A-Z


  found = 0;

  for (byte i = 0; i < 62; i++) {
    if (isTaken(i)) {
      found++;
    }
  }

  if (!found) {
    //Serial.println("No DECAGON 5TE sensors found.");
  } // stop here

  // String foundstr = "Found ";
  // foundstr += String(found, DEC);
  // foundstr += " Decagon 5TE sensors.";
  //Serial.println(foundstr);
  //Serial.println(found);
}

void SmartFarmMeasure::readDecSensors(int numDecSens) {
  String result = "";
  String NA = "NA";
  int placeholder = MaxDecSens - numDecSens; // 3-count, expected value = 3-3,3-2,3-1,3-0 = 0,1,2,3 NAs to print
  // result ="Reading Decagon 5TE Sensors... \n";
  // result += "VWC(Dielectric) EC(dS/m) Temperature(Deg C)\n";

  for (char i = '0'; i <= '9'; i++) if (isTaken(i)) {

	  //printInfo(i);        //Prints "#13DECAGON 5TE   365." which is "Sensor ID, Type, and Version."
      takeDecMeasurement(i);        //This prints " 1.01 0.00 24.7"
      smartSDI12.flush();
      //Serial.print(result);

    }

  // scan address space a-z
  for (char i = 'a'; i <= 'z'; i++) if (isTaken(i)) {
      // result += " D" + String(i); //should be the id of the dec sensor  printing "# "
      	// write2SD(result); //write
		// Serial.print(result); //print
		// result = "";
	  //printInfo(i);        //Prints "#13DECAGON 5TE   365." which is "Sensor ID, Type, and Version."
      takeDecMeasurement(i);        //This prints " 1.01 0.00 24.7"
      smartSDI12.flush();
      //Serial.print(result);
    }

  // scan address space A-Z
  for (char i = 'A'; i <= 'Z'; i++) if (isTaken(i)) {
      // result += " D" + String(i); //should be the id of the dec sensor  printing "# "
      	// write2SD(result); //write
		// Serial.print(result); //print
		// result = "";
	  //printInfo(i);        //Prints "#13DECAGON 5TE   365." which is "Sensor ID, Type, and Version."
      takeDecMeasurement(i);        //This prints " 1.01 0.00 24.7"
      smartSDI12.flush();
      // Serial.print(result);
    }

  for (int i = 0; i < placeholder; i++) {
    result += " D# " + NA + ' ' + NA + ' ' + NA; //should print D# NA NA NA as a placeholder for sensors
	}
	write2SD(result); //write
	Serial.print(result); //print
}

void SmartFarmMeasure::takeDecMeasurement(char i) {
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
  printBufferToScreen(i); 
}

void SmartFarmMeasure::printBufferToScreen(char i) { // prints the data 1.22 0.00 26.5
  String buffer = " D" + String(i); //should be the id of the dec sensor  printing "# "
  smartSDI12.read(); // consume address
  while (smartSDI12.available()) {
    char c = smartSDI12.read();
    if (c == '+' || c == '-') {
      buffer += ' ';  // this may need to be changed back
      if (c == '-') buffer += '-';
    }
    else if (c == '\r' || c == '\n') {
        buffer +="";
		// break;
    }
	else {
      buffer += String(c);
	  //if (c == '\n') buffer += "NEWLINE CHARACTER";
    }
    delay(100);
  }
  write2SD(buffer); //write
  Serial.print(buffer); //print
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
  i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61.
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
      if (c == '1') {
        break; //break after the id
      }
    }
    delay(5);
  }
}

// converts allowable address characters '0'-'9', 'a'-'z', 'A'-'Z',
// to a decimal number between 0 and 61 (inclusive) to cover the 62 possible addresses
byte SmartFarmMeasure::charToDec(char i) {
  if ((i >= '0') && (i <= '9')) return i - '0';
  if ((i >= 'a') && (i <= 'z')) return i - 'a' + 10;
  if ((i >= 'A') && (i <= 'Z')) return i - 'A' + 37;
}

// THIS METHOD IS UNUSED IN THIS EXAMPLE, BUT IT MAY BE HELPFUL.
// maps a decimal number between 0 and 61 (inclusive) to
// allowable address characters '0'-'9', 'a'-'z', 'A'-'Z',
char SmartFarmMeasure::decToChar(byte i) {
  if ((i >= 0) && (i <= 9)) return i + '0';
  if ((i >= 10) && (i <= 36)) return i + 'a' - 10;
  if ((i >= 37) && (i <= 62)) return i + 'A' - 37;
}

void SmartFarmMeasure::setupWM() {
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  delay(80);
  digitalWrite(6, HIGH);
  delay(80);
  digitalWrite(6, LOW);
  pinMode(6, INPUT);
  //Set up the power port
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

void SmartFarmMeasure::finishUp() {
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  delay(80);
  digitalWrite(6, HIGH);
  delay(80);
  digitalWrite(6, LOW);
  pinMode(6, INPUT);
}

void SmartFarmMeasure::readWM() {
  // Scan watermark connections
  float WMreadings[4] = { -1, -1, -1, -1}; // initialized watermark sensor readings
  byte WC = 0B00000000; //watermark connection check
  String WMdata = "";
  digitalWrite(4, HIGH); // sets the pin on
  digitalWrite(5, LOW); // sets the pin off
  delayMilliseconds(5);

  for (int i = 0; i <= 3; i++) // Go through all 4 ports
  {
    if (analogRead(i) < 750) //3.3V for 758
    {
      WC |= 1 << i;
    }
  }
  digitalWrite(4, LOW); // sets the pin on
  digitalWrite(5, HIGH); // sets the pin off
  delayMilliseconds(5);

  for (int i = 0; i < 4; i++) // Go through each port to read data
  {
    String Rstring = "";
    if ((WC & (1 << i)) == (1 << i) ) // Sensor connected at port i?
    {
      float Rs = 0.0; //sensor resistance
      float v1 = 0; //volt measurement 1
      float v2 = 0; //volt measurement 1
      for (int k = 0; k < 5; k++) // Take measurements
      {
        digitalWrite(4, HIGH); // sets the pin on
        digitalWrite(5, LOW); // sets the pin off
        delayMilliseconds(5); // 100 Hz
        v1 = analogRead(i) * 3.3 / 758.0;
        delay(100);
        digitalWrite(4, LOW); // sets the pin on
        digitalWrite(5, HIGH); // sets the pin off
        delayMilliseconds(5); // 100 Hz
        v2 = analogRead(i) * 3.3 / 758.0;
        delay(100);
        if (v2 <= 0.0) v2 = 0.1;
        if (v1 <= 0.0) v1 = 0.1;

        float Rs1 = 10000.0 * v1 / (3.3 - v1); //read R
        float Rs2 = 10000.0 * 3.3 / v2 - 10000.0; //read R again
        Rs = (Rs1 + Rs2) / 2.0 + Rs; //take an average and sum up
      }
      float R = Rs / 5.0;
      if ((R > 0.0) && (R < 99000.0)) //limit of resistance
        Rstring = String(R, 3);
      else
        Rstring = "NA";
    } else {
      Rstring = "NA";
    }
    WMdata = WMdata + Rstring + " "; // resistor reading for sensor i
  }
  digitalWrite(4, LOW); // Shut down the watermark power
  digitalWrite(5, LOW);
  write2SD(WMdata); //write
  Serial.print(WMdata); //print
}

void SmartFarmMeasure::delayMilliseconds(int x) {
  for (int i = 0; i < x; i++)
  {
    delayMicroseconds(1000);// 1 millsec
  }
}