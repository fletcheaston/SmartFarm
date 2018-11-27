#Created by Fletcher Easton

import data_logger;

import serial;
from time import sleep;

import sys;

def main():
    data_logger.log_action("Attempting to restart logger script. Issues may occur if script is already running as a seperate process. Contact Fletcher Easton if you need assistance or if unknown errors occur.");

    baudrate = 57600;
    port = "/dev/ttyUSB0";

    try:
        reader = serial.Serial(port,baudrate);
        data_logger.log_action("Reopening the serial reader.");
    except:
        data_logger.log_action("Error opening port: {!r}".format(port));
        sys.exit();

    sleep(1); #Allows the serial connection to finish setting up

    reader.flushInput();
    reader.flushOutput();

    while(True):
        try:
            while(reader.in_waiting):
                serial_string = reader.readline().decode('utf-8').strip();

                data_logger.log_action("Read string from serial:" + serial_string);

                data_logger.log_raw_data(serial_string);

        except Exception as e:
            data_logger.log_action("An error has occured.");
            data_logger.log_action(e);

main();
