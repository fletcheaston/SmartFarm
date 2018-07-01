#Created by Fletcher Easton

#import fresnode;
import data_logger;

import serial;
from time import sleep;

import sys;

def main():
	baudrate = 57600;
	port = "/dev/ttyUSB0";

	try:
		reader = serial.Serial(port,baudrate);
	except:
		data_logger.log_action("Error opening port: {!r}".format(port));
		sys.exit();

	sleep(1); #Allows the serial connection to finish setting up

	reader.flushInput();
	reader.flushOutput();

	while(True):

		while(reader.in_waiting):
			serial_string = reader.readline().decode().strip();

			data_logger.log_action("Read string from serial.");
			data_logger.log_raw_data(serial_string);

main();
