#ifndef SmartFarmMeasureDAQv6_1_H
#define SmartFarmMeasureDAQv6_1_H

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
    bool checkSafeSDVolts();
		//void readTemps(String boardID);
		// String readTemps(String boardID);
		String readTemps(int count);
		void setupSD();
		void write2SD(String dataString);
		void setupDecSensors();
		String readDecSensors();
		void setupWM();
		void test2_setupWM();
		//void readWM(String boardID);
		String readWM();
		String test_readWM(String boardID);
		String test2_readWM(String boardID);
		void setupMUXAnalog();
		String readMUXAnalog(String boardID);
		void selectMuxPin(byte pin);
		void setupAll();
		void runAll(String boardID);
		//void printUpload(String boardID);
		String printUpload(String boardID);
		String timeStamp();
		String readVolts();
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
