//2/10/18
//Adapted for circuit DAQ v6.2
#ifndef SmartFarmMeasureDAQv6_2_H
#define SmartFarmMeasureDAQv6_2_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h> //Decagon Sensor Library
#include <SDI12.h> //Decagon Sensor Library
#include <Arduino.h>
 

class SmartFarmMeasure {
	public:
		//SmartFarmMeasure();
		//*******Node functions*******
		void finishUp(); //wireless programming completed indicator to MCU from DAQ
		
		//*******Node hardware setup functions*******
		//RTC
		void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
		void readDS3231time(byte *second,//read rtc 
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year);
		byte bcdToDec(byte val);//used by RTC
		byte decToBcd(byte val);//used by RTC
		void setRTCToComputerTime(char myDATEString[], char myTIMEString[]); //sets RTC to pc time on upload
		String timeStamp(String boardID);//read time and pass as a string
		//battery and solar voltage
		String readVolts(String boardID);//read voltages and pass as a string
		//SD Card
		void setupSD();//setup SD card slot
		void write2SD(String dataString);//write to sd card
		//analog mux circuit
		void selectMuxPin(byte pin); //selsets the mux pin to read
		//*******sensor setup functions*******
		void setupAll();//sets up all sensors
		//watermark
		void setupWM();//setup watermark sensors
		void test2_setupWM();//setup wm, Same as v6.2 setupWM?
		//DS18B20 temperature
		void setupTemps();//setup temperature sensors
		//Decagon
		//void setupDecSensors();//setup dec sensors
		String setupDecSensors(String boardID); //setup dec sensors
		
		//*******sensor read functions*******
		void runAll(String boardID);// reads all sensors
		//watermark
		String readWM(String boardID, int WMpos1, int WMpos2, int WMpos3);//read wm per position and pass as a string
		//DS18B20 temperature
		//void readTemps(String boardID); 
		//String readTemps(String boardID);
		String readTemps(String boardID, int tempPos1, int tempPos2, int tempPos3);
		//Decagon
		//void readDecSensors(String boardID);// read decagon sensors
		String readDecSensors(String boardID); // read decagon sensors
		//void printUpload(String boardID);	
		String printUpload(String boardID);	//prints "Upload" for WebIDE uploadiong on next wakeup; changed to NPROG begin/end

	private:
		//DS18B20 temperature
		String build_data_string(float* data, int numtempsens, String results);//used for temp sensors
		String id_builder(String boardID, int tempPos1, int tempPos2, int tempPos3);//used for temp sensors
		void get_temp(float* data, int numtempsens, int tempPos1, int tempPos2, int tempPos3); //used for temp sensors
		String getDevAddress(DeviceAddress deviceAddress);//used for temp sensors
		//void printData(DeviceAddress deviceAddress);
		//String printData(DeviceAddress deviceAddress);
		//Decagon
		//void takeDecMeasurement(char i);
		String takeDecMeasurement(char i); //decagon measurements
		//void printBufferToScreen(char i);
		boolean checkActive(char i);//checks decagon activity
		boolean setTaken(byte i);//places decagon in addressRegister 
		boolean setVacant(byte i);//sets vacant in addressRegister
		boolean isTaken(byte i);// checks addressRegister for active sensor
		char printInfo(char i);// identification information from a sensor
		byte charToDec(char i);//converts allowable address characters to 0-61
		char decToChar(byte i);//maps 0-61 to allowable address characters
		void delayMilliseconds(int x);//keeps time
};

#endif