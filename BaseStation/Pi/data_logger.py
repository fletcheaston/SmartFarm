#Created by Fletcher Easton

import datetime;
import os;
import shutil;
import sys;

# Used to record the raw data from the serial reader.
raw_data_log = "/var/log/smarfarm/SmartFarmData.log";

# Used to record each action of the scripts. Contains all data from the log_action(string) function.
# Actions that should be logged include "reading serial data", "parsing serial data", errors, etc.
action_log = "action_log.txt";
previous_action_log = "previous_action_log.txt";

# Defines the maximum size of the action log, in bytes. This is arbitrarily set to prevent the system from filling up with action logs.
# Some basic math time! A full set of readings on a single node (version 6.2, deployment Fx) measures, at most, 200 bytes in size. We've got eight of these nodes, and two nodes from the Bx deployment. Each node from the Bx deployment gives a full set of readings that measure, at most, 218 bytes. A full set of readings from all boards measures 2,036 bytes, at most. Readings are taken approximately every ten minutes, which gives 144 sets of readings every day. In total, every day accounts for 293,184 bytes of data written to the action log, at most.
# That's a lot of data! And this is with a small deployment, a larger deployment will greatly increase the amount of data written to the action log. Our Raspberry Pi has a few gigabytes of free space, but no need to fill it up with junk. So every few days, we should overwrite the data.
B_max_data = 218;
F_max_data = 200;

B_board_count = 2;
F_board_count = 8;

# Measured in minutes, amount of time from one set of readings to another.
B_reading_delay = 10;
F_reading_delay = 10;

# Hours per day * minutes per hour.
minutes_per_day = 24 * 60;

B_readings_per_day = minutes_per_day // B_reading_delay;
F_readings_per_day = minutes_per_day // F_reading_delay;

B_all_readings_per_day = B_max_data * B_board_count * B_readings_per_day;
F_all_readings_per_day = F_max_data * F_board_count * F_readings_per_day;

# Give it a full week until data overwrite.
days_until_overwrite = 7;

# Approximately 2MB of data for a full week.
max_log_size = (B_all_readings_per_day + F_all_readings_per_day) * days_until_overwrite;

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

		if(os.path.getsize(action_log) > max_log_size):
			reset_action_log();
			log_action("Archiving and resetting the action log.");

	except:
		print("Error writing data ({!r}) to action log. Fix immediately.".format(string));

# Archives the current action log and erases it's contents.
# Currently, approximately two weeks of actions are logged and archived. After this, the log is erased. Two weeks should be plenty of time to determine if something has gone wrong with the serial reader and/or data logger scripts.
def reset_action_log():
	source_file = open(action_log, "r");
	target_file = open(previous_action_log, "w");

	shutil.copyfileobj(source_file, target_file);

	erased_action_log = open(action_log, "w").close();
