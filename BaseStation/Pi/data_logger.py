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
action_log_archive = "/home/pi/SmartFarm_CurrentDAQ/action_log_archive.txt";

# Defines the maximum size of the action log, in bytes. This is arbitrarily set to prevent the system from filling up with action .
max_log_size = 2000000;

def current_timestamp():
	temp = datetime.datetime.now();
	return("{:04}/{:02}/{:02}/{:02}:{:02}:{:02}".format(temp.year,temp.month,temp.day,temp.hour,temp.minute,temp.second));

def log_raw_data(string):
	try:
		with open(raw_data_log, "a") as file:
			line = current_timestamp() + " | " + string + "\n";
			file.write(line);
			log_action("Wrote raw data to log.");

	except:
		log_action("Unable to write raw data ({!r}) to log.".format(string));

# Log each action to a file. Helps to keep track of errors.
def log_action(string):
	try:
		with open(action_log, "a") as file:
			line = current_timestamp() + " | " + str(string) + "\n";
			file.write(line);
			print(line);

		# Checks if the action log is too large. If so, the log is archived and erased.
		check_for_log_reset();

	except:
		print("Error writing data ({!r}) to action log. Fix immediately.".format(string));

# Checks if the action log is too large. If so, the log is archived and erased.
def check_for_log_reset():
	if(os.path.getsize(action_log) > max_log_size):
		reset_action_log();
		log_action("Archiving and resetting the action log.");

# Archives the current action log and erases it's contents.
# Currently, approximately two weeks of actions are logged and archived. After this, the log is erased. Two weeks should be plenty of time to determine if something has gone wrong with the serial reader and/or data logger scripts.
def reset_action_log():
	source_file = open(action_log, "r");
	target_file = open(action_log_archive, "w");

	# Copies the contents of the source file into the target file, and closes both files.
	shutil.copyfileobj(source_file, target_file);

	# Reopens the action log in write mode, and immediately closes it. This erases the action log.
	open(action_log, "w").close();
