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
		String readTemps(int count);
		void setupSD();
		void write2SD(String dataString);
		void setupDecSensors();
		String readDecSensors();
		void setupWM();
		String readWM(int pins[],int count);
		void selectMuxPin(byte pin);
		void setupAll();
		String timeStamp();
		String readVolts();
		bool checkSafeSDVolts();
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
