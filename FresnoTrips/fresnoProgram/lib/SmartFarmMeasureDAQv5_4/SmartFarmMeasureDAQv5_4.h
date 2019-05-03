#ifndef SmartFarmMeasure_H
#define SmartFarmMeasure_H

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
		void setupAll();
		void setupSD();
		void write2SD(String dataString);
		void setupWM();
		void setupTemps();
		void setupDecSensors();
		//void numDecsens(int number);
		void runAll();
		void readWM();
		void readTemps();
		void readDecSensors(int numDecSens);	
		
		void printUpload();		
		void finishUp();	
	private:
		String getDevAddress(DeviceAddress deviceAddress);
		void printData(DeviceAddress deviceAddress);
		void takeDecMeasurement(char i);
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