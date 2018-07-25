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

		// Some Low Level Functions

		void finishUp();

		// RTC Functions

		void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);

		void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);

		void setRTCToComputerTime(char myDATEString[], char myTIMEString[]);

		String timeStamp();

		// Voltage Functions

		String readVolts();

		// SD Card Functions

		void setupSD();

		void write2SD(String dataString);

		// Setup Sensor Functions

		void setupAll();

		void setupWM();

		void setupTemps();

		void setupDecSensors();

		// Read Sensor Functions

		String readWM(int count);

		String readTemps(int count);

		String readDecSensors();

	private:
		// Most of the Low Level Functions

		void SmartFarmMeasure();

		byte decToBcd(byte val);

		byte bcdToDec(byte val);

		byte charToDec(char i);

		char decToChar(byte i);

		boolean checkActive(char i);

		boolean setTaken(byte i);

		boolean setVacant(byte i);

		boolean isTaken(byte i);

		void selectMuxPin(byte pin);

		String getDevAddress(DeviceAddress device_address);

		String takeDecMeasurement(char i);

		void delayMilliseconds(int x);

};

#endif
