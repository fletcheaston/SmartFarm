#Created by Fletcher Easton

import datetime;
import os;
import shutil;
import sys;

# Used to record the raw data from the serial reader.
raw_data_log = "/var/log/smartfarm/SmartFarmData.log";

# Used to record each action of the scripts. Contains all data from the log_action(string) function.
# Actions that should be logged include "reading serial data", "parsing serial data", errors, etc.
action_log = "/home/pi/SmartFarm_CurrentDAQ/action_log.txt";

def current_timestamp():
    temp = datetime.datetime.now();
    return("{:04}/{:02}/{:02}/{:02}:{:02}:{:02}".format(temp.year,temp.month,temp.day,temp.hour,temp.minute,temp.second));

def log_raw_data(string):
    try:
        with open(raw_data_log, "a") as file:
            line = current_timestamp() + " | " + string + "\n";
            file.write(line);

        log_action("Wrote raw data to log.");
        
        print("Logged data: {!r}".format(line));

    except:
        log_action("Unable to write raw data ({!r}) to log.".format(string));

# Log each action to a file. Helps to keep track of errors.
def log_action(string):
    try:
        with open(action_log, "a") as file:
            line = current_timestamp() + " | " + str(string) + "\n";
            file.write(line);

        print(line);
    except:
        print("Error writing data ({!r}) to action log. Fix immediately.".format(string));
