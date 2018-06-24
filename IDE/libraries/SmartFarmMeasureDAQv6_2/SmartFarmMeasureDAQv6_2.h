//2/10/18
//Adapted for circuit DAQ v6.2
#ifndef SmartFarmMeasureDAQv6_2_H
#define SmartFarmMeasureDAQv6_2_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <SDI12.h>
#include <Arduino.h>


class SmartFarmMeasure {
	public:
		SmartFarmMeasure();
		void finishUp();
		void setupTemps();
		//void readTemps(String boardID);
		//String readTemps(String boardID);
		String readTemps(String boardID, int tempPos1, int tempPos2, int tempPos3);
		void setupSD();
		void write2SD(String dataString);
		void setupDecSensors();
		void readDecSensors(int numDecSens, String boardID);
		void setupWM();
		void test2_setupWM();
		//void readWM(String boardID);
		String readWM(String boardID);//integrated mux circuit
		String test_readWM(String boardID);
		String test2_readWM(String boardID);
		//void setupMUXAnalog(); deprecated for v6.2 and combined into setupMuxWM
		//String readMUXAnalog(String boardID); deprecated for v6.2 and combined into setupMuxWM
		String readWM_West(String boardID);
		String readWM_East(String boardID);
		void selectMuxPin(byte pin);
		void setupAll();
		void runAll(String boardID);
		//void printUpload(String boardID);
		String printUpload(String boardID);
		String timeStamp(String boardID);
		String readVolts(String boardID);
		void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year);
		void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
		byte bcdToDec(byte val);
		byte decToBcd(byte val);
		void setRTCToComputerTime(char myDATEString[], char myTIMEString[]);
	private:
		String getDevAddress(DeviceAddress deviceAddress);
		//void printData(DeviceAddress deviceAddress);
		//String printData(DeviceAddress deviceAddress);
		//void takeDecMeasurement(char i);
		String takeDecMeasurement(char i);
		void printBufferToScreen(char i);
		boolean checkActive(char i);
		boolean setTaken(byte i);
		boolean setVacant(byte i);
		boolean isTaken(byte i);
		char printInfo(char i);
		byte charToDec(char i);
		char decToChar(byte i);
		void delayMilliseconds(int x);
};

#endif
