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
		String timeStamp();//read time and pass as a string
		//battery and solar voltage
		String readVolts();//read voltages and pass as a string
		//SD Card
		void setupSD();//setup SD card slot
		void write2SD(String dataString);//write to sd card
		//analog mux circuit
		void selectMuxPin(byte pin); //selsets the mux pin to read
		//*******sensor setup functions*******
		void setupAll();//sets up all sensors
		//watermark
		void setupWM();//setup watermark sensors
		void setupTemps();//setup temperature sensors

		//*******sensor read functions*******
		String readWM(int count);
		String readTemps(int count);

	private:
		void get_temp(float* data, int numtempsens, int tempPos1, int tempPos2, int tempPos3); //used for temp sensors
		void delayMilliseconds(int x);//keeps time
};

#endif
