#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <SDI12.h>

//Needed variables for temp sensors
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 9
#define MAX_SENSORS 4
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
DeviceAddress therms[MAX_SENSORS];
static int numsens = 0;

//Needed variables for Decagon sensors
#define DEC_PIN 2
SDI12 smartSDI12(DEC_PIN);
byte addressRegister[8] = { 
  0B00000000, 
  0B00000000, 
  0B00000000, 
  0B00000000, 
  0B00000000, 
  0B00000000, 
  0B00000000, 
  0B00000000 
}; 

//Needed variables for Watermark sensors
String WMdata = ""; 
const int chipSelect = 10;

void setup() {
  Serial.begin(57600);
  setupWM();
  delay(1000);
  setupTemps();
  delay(1000);
  setupDecSensors();
}

void loop() {
  readWM();
  delay(1000);
  readTemps();
  delay(1000);
  readDecSensors();
  delay(10000);
}

void setupSD() {
  SD.begin(chipSelect);
}

void write2SD(String dataString)
{
  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) // if the file is available, write to it:
  { dataFile.println(dataString);
    dataFile.close();
    Serial.println("\nSD Card written.");
  }
  else // if the file isn't open, pop up an error:
  { 
    //Serial.println("\nSD Card not found.");
  }
  
}

void readTemps() {
  Serial.println("Reading temperature sensors...");
  sensors.requestTemperatures();

  // print the device information
  for (int i = 0; i < numsens; i++) {
    printData(therms[i]);
  }
}

void setupTemps() {
  String initstr = "Finding temperature sensors: Found ";
  String toprint = "";
  sensors.begin();
  
  numsens = sensors.getDeviceCount();
  initstr += String(numsens, DEC);
  initstr += " devices.";
  Serial.println(initstr);
  write2SD(initstr);
  
  for (int i = 0; i < numsens; i++)
    if (!sensors.getAddress(therms[i], i)) Serial.println("Unable to find address for Device " + i); 

  for (int i = 0; i < numsens; i++) {
    toprint = "Device Address ";

    toprint += String(i, DEC);
    toprint += ": ";
    toprint += getDevAddress(therms[i]);
    sensors.setResolution(therms[i], TEMPERATURE_PRECISION);
    toprint += '\n';
    toprint += "Device Resolution ";
    toprint += String(i, DEC);
    toprint += ": ";
    toprint += String(sensors.getResolution(therms[i]), DEC);
    toprint += "\n";
    Serial.println(toprint);
  }
  //Serial.println(toprint);
  write2SD(toprint);
}

String getDevAddress(DeviceAddress deviceAddress)
{
  String result = "";
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) 
      result+= '0';
    result += String(deviceAddress[i], HEX);
  }
  return result;
}


void printData(DeviceAddress deviceAddress)
{
  String results = "Device Address: ";
  float tempC;

  results += getDevAddress(deviceAddress);
  results += " ";
  tempC = sensors.getTempC(deviceAddress);
  results += "Temp C: ";
  results += tempC;
  results += " Temp F: ";
  results += DallasTemperature::toFahrenheit(tempC);

  Serial.println(results);
  

}

void readDecSensors() {

  Serial.println("Reading Decagon 5TE Sensors... ");
  Serial.println("Format: (Time elapsed), (Sensor ID, Type, and Version). (Measurments)");  
  for(char i = '0'; i <= '9'; i++) if(isTaken(i)){
    Serial.print(millis()/1000);
    Serial.print(", ");
    printInfo(i);   
    takeDecMeasurement(i);
  }

  // scan address space a-z
  for(char i = 'a'; i <= 'z'; i++) if(isTaken(i)){
    Serial.print(millis()/1000);
    Serial.print(", ");
    printInfo(i);   
    takeDecMeasurement(i);
  } 

  // scan address space A-Z
  for(char i = 'A'; i <= 'Z'; i++) if(isTaken(i)){
    Serial.print(millis()/1000);
    Serial.print(", ");
    printInfo(i);   
    takeDecMeasurement(i);
  };   
  
}

void setupDecSensors() {
  smartSDI12.begin(); 
  delay(500); // allow things to settle

  Serial.println("Scanning Decagon 5TE addresses."); 
  write2SD("Scanning Decagon 5TE addresses.");

  for(byte i = '0'; i <= '9'; i++) if(checkActive(i)) setTaken(i);   // scan address space 0-9

  for(byte i = 'a'; i <= 'z'; i++) if(checkActive(i)) setTaken(i);   // scan address space a-z

  for(byte i = 'A'; i <= 'Z'; i++) if(checkActive(i)) setTaken(i);   // scan address space A-Z


  int found = 0; 

  for(byte i = 0; i < 62; i++){
    if(isTaken(i)){
      found++;
    }
  }

  if(!found) {
    Serial.println("No DECAGON 5TE sensors found."); 
  } // stop here
  
  String foundstr = "Found ";
  foundstr += String(found, DEC);
  foundstr += " Decagon 5TE sensors.";
  Serial.println(foundstr);
  write2SD(foundstr);
}

void takeDecMeasurement(char i){
  String command = ""; 
  command += i;
  command += "M!"; // SDI-12 measurement command format  [address]['M'][!]
  smartSDI12.sendCommand(command); 
  while(!smartSDI12.available()>5); // wait for acknowlegement with format [address][ttt (3 char, seconds)][number of measurments available, 0-9]
  delay(100); 
  
  smartSDI12.read(); //consume address
  
  // find out how long we have to wait (in seconds).
  int wait = 0; 
  wait += 100 * smartSDI12.read()-'0';
  wait += 10 * smartSDI12.read()-'0';
  wait += 1 * smartSDI12.read()-'0';
  
  smartSDI12.read(); // ignore # measurements, for this simple examlpe
  smartSDI12.read(); // ignore carriage return
  smartSDI12.read(); // ignore line feed
  
  long timerStart = millis(); 
  while((millis() - timerStart) > (1000 * wait)){
    if(smartSDI12.available()) break;                //sensor can interrupt us to let us know it is done early
  }
  
  // in this example we will only take the 'DO' measurement  
  smartSDI12.flush(); 
  command = "";
  command += i;
  command += "D0!"; // SDI-12 command to get data [address][D][dataOption][!]
  smartSDI12.sendCommand(command);
  while(!smartSDI12.available()>1); // wait for acknowlegement  
  delay(300); // let the data transfer
  printBufferToScreen(); 
  smartSDI12.flush(); 
}

void printBufferToScreen(){
  String buffer = "";
  Serial.print(". Measurements follow");
  write2SD(". Measurements follow");
  smartSDI12.read(); // consume address
  while(smartSDI12.available()){
    char c = smartSDI12.read();
    if(c == '+' || c == '-'){
      buffer += ", ";   
      if(c == '-') buffer += '-'; 
    } 
    else {
      buffer += c;  
    }
    delay(100); 
  }
 Serial.print(buffer);
 write2SD(buffer);
}


// this checks for activity at a particular address     
// expects a char, '0'-'9', 'a'-'z', or 'A'-'Z'
boolean checkActive(char i){              

  String myCommand = "";
  myCommand = "";
  myCommand += (char) i;                 // sends basic 'acknowledge' command [address][!]
  myCommand += "!";

  for(int j = 0; j < 3; j++){            // goes through three rapid contact attempts
    smartSDI12.sendCommand(myCommand);
    if(smartSDI12.available()>1) break;
    delay(30); 
  }
  if(smartSDI12.available()>2){       // if it hears anything it assumes the address is occupied
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
boolean setTaken(byte i){          
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
boolean setVacant(byte i){
  boolean initStatus = isTaken(i);
  i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61. 
  byte j = i / 8;   // byte #
  byte k = i % 8;   // bit #
  addressRegister[j] &= ~(1 << k); 
  return initStatus; // return false if already vacant
}


// this quickly checks if the address has already been taken by an active sensor           
boolean isTaken(byte i){         
  i = charToDec(i); // e.g. convert '0' to 0, 'a' to 10, 'Z' to 61. 
  byte j = i / 8;   // byte #
  byte k = i % 8;   // bit #
  return addressRegister[j] & (1<<k); // return bit status
}

// gets identification information from a sensor, and prints it to the serial port
// expects a character between '0'-'9', 'a'-'z', or 'A'-'Z'. 
char printInfo(char i){
  int j; 
  String command = "";
  String result = "";
  command += (char) i; 
  command += "I!";
  for(j = 0; j < 1; j++){
    smartSDI12.sendCommand(command);
    delay(30); 
    if(smartSDI12.available()>1) break;
    if(smartSDI12.available()) smartSDI12.read(); 
  }

  while(smartSDI12.available()){
    char c = smartSDI12.read();
    if((c!='\n') && (c!='\r')) {
      Serial.write(c);
      result += c;
    }
    delay(5); 
  } 
  write2SD(result);
}

// converts allowable address characters '0'-'9', 'a'-'z', 'A'-'Z',
// to a decimal number between 0 and 61 (inclusive) to cover the 62 possible addresses
byte charToDec(char i){
  if((i >= '0') && (i <= '9')) return i - '0';
  if((i >= 'a') && (i <= 'z')) return i - 'a' + 10;
  if((i >= 'A') && (i <= 'Z')) return i - 'A' + 37;
}

// THIS METHOD IS UNUSED IN THIS EXAMPLE, BUT IT MAY BE HELPFUL. 
// maps a decimal number between 0 and 61 (inclusive) to 
// allowable address characters '0'-'9', 'a'-'z', 'A'-'Z',
char decToChar(byte i){
  if((i >= 0) && (i <= 9)) return i + '0';
  if((i >= 10) && (i <= 36)) return i + 'a' - 10;
  if((i >= 37) && (i <= 62)) return i + 'A' - 37;
}

void setupWM() {
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

void readWM() {
  // Scan watermark connections
  float WMreadings[4] = { -1, -1, -1, -1}; // initialized watermark sensor readings
  byte WC = 0B00000000; //watermark connection check
  WMdata = "";
  Serial.println("Reading Watermark sensors...");
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
      float Rs = 0.0; //sensor reasistance
      float v1 = 0; //volt measurement 1
      float v2 = 0; //volt measurement 1
      for (int k = 0; k < 5; k++) // Take measurements
      {
        digitalWrite(4, HIGH); // sets the pin on
        digitalWrite(5, LOW); // sets the pin off
        delayMilliseconds(5); // 100 Hz
        v1 = analogRead(i) * 3.3 / 758.0;

        digitalWrite(4, LOW); // sets the pin on
        digitalWrite(5, HIGH); // sets the pin off
        delayMilliseconds(5); // 100 Hz
        v2 = analogRead(i) * 3.3 / 758.0;
        if (v2 <= 0.0) v2 = 0.1;
        if (v1 <= 0.0) v1 = 0.1;

        float Rs1 = 10000.0 * v1 / (3.3 - v1); //read R
        float Rs2 = 10000.0 * 3.3 / v2 - 10000.0; //read R again
        Rs = (Rs1 + Rs2) / 2.0 + Rs; //take an average and sum up
      }
      float R = Rs / 5.0;
      if (R > 0.0)
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

  // Save readings
  Serial.println(WMdata);
  write2SD(WMdata);
}

void delayMilliseconds(int x) {
  for (int i = 0; i < x; i++)
  {
    delayMicroseconds(1000);// 1 millsec
  }
}
