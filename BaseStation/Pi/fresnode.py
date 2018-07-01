# Created by Fletcher Easton
# As all nodes are located in Fresno, the name FresNode seemed to make the most sense.

import datetime;

class FresNode:
	def __init__(id,timestamp,voltage_data=None,watermark_data=None,temperature_data=None,decagon_data=None):
		self.id = id;
		# Typically a letter followed by a number. The letter identifies which phase the board was deployed during, the number identifies which board in that phase the data relates to
		# EX: id = "B2". "B" boards were all deployed at the same time, the letter itself is arbitrarily set. The "2" points to a specific board in the "B" deployment.
		# A string.

		self.timestamp = timestamp;
		# These should all be ints.

		self.voltage_data = voltage_data;
		self.watermark_data = watermark_data;
		self.temperature_data = temperature_data;
		self.decagon_data = decagon_data;
		# These should each be a lists of floats.

	def __repr__(self):
		return("Node ID: {!r} | Timestamp: {!r}/{!r}/{!r}/{!r}:{!r}:{!r} | Watermark Data: {!r} | Temperature Data: {!r} | Decagon Data: {!r}".format(self.id,self.year,self.month,self.day,self.hour,self.minute,self.second,self.watermark_data,self.temperature_data,self.decagon_data));

	# A standard built-in compare function for comparing two FresNode objects. Used for sorting.
	def __cmp__(self, other):
		if(type(self) == type(other)):

			# First, we sort by node id. This lets us seperate each node from the other. Typically, nodes will already be seperated into their respective lists, but for sorting all of the node data at once, this is a solid start.
			if(self.id != other.id):
				return((self.id).__cmp__(other.id));

			# Then, we sort by the timestamp. Useful for sorting large chunks of data from several days/weeks of measurements.
			return((self.timestamp).__cmp__(other.timestamp));

################################################################################

def combine_listed_node_data(node_list):
	# Sorts the node list by first their IDs
	node_list = sorted(node_list);

	while(True):
		finished = True;

		for i in range(len(node_list) - 1):
			node_list[i], node_list[i + 1] = combine_node_data(node_list[i], node_list[i + 1]);
			if(node_list[i + 1] == None):
				del(node_list[i + 1]);
				i -= 1;
				finished = False;

		if(finished):
			break;

	return(node_list);

# Used to combine the data from two different readings. Typically used to add decagon data from one reading to another. Any data that the node_a object is missing is added from the node_b object.
def combine_node_data(node_a,node_b):
	if(node_a.id == node_b.id):
		if(node_a.voltage_data == None):
			node_a.voltage_data = node_b.voltage_data;
		if(node_a.watermark_data == None):
			node_a.watermark_data = node_b.watermark_data;
		if(node_a.temperature_data == None):
			node_a.temperature_data = node_b.temperature_data;
		if(node_a.decagon_data == None):
			node_a.decagon_data = node_b.decagon_data;

		# Returns the node_a, which contains any data it doesn't originally have from node_b.
		# node_b is no longer needed, and returned as None.
		return(node_a,None);
	else:
		# Unable to combine the two nodes, as their IDs are different. Returns both nodes.
		return(node_a,node_b);

################################################################################

# list of FresNode objects -> a new list of FresNode objects.
# Sorts the list of FresNode objects by their chronological order.
# See above in the FresNode class for more details on comparing objects.
# sorted(list) returns a new list while list.sort() sorts the list in place.
def sort_fresnodes(node_list):
	sorted_list = sorted(node_list);
	return(sorted_list);

################################################################################

# list of FresNode objects -> list of lists of FresNode objects.
# Creates a seperate list for each FresNode ID, and puts each FresNode object into it's respective list.
def seperate_fresnodes(node_list):
	# First, creates a list of different FresNode IDs.
	id_list = unique_ids(node_list);

	# Then, sorts the ID list in alphanumeric order.
	# Not necessary in the slightest, just makes the data a bit prettier.
	id_list.sort();

	# Then, creates a list of lists to seperate each FresNode into by ID.
	sepearted_node_list = [[] for i in range(count)];

	# Then, determines which inner list to append each node to.
	for node in node_list:
		index = id_list.index(node.id);
		sepearted_node_list[index].append(node);

	# Finally, returns the newly sepearated list of lists.
	return(sepearted_node_list);

################################################################################

# list of FresNode objects -> list of unique IDs strings.
# Creates a list containing the various unique IDs
def unique_ids(node_list):
	# An empty list to append IDs.
	id_list = [];

	# Searches through the full node list for unique IDs, and adds new IDs to the list of unique IDs.
	for node in node_list:
		if(node.id not in id_list):
			id_list.append(node.id);

	# And obviously returns the unique ID list.
	return(id_list);

################################################################################
# Currently unused parsing code. May be implemented in the future for more complex/modular parsing.

'''

# string -> FresNode object
# Takes a string and attempts to interpret it as a FresNode object.
def parse_into_fresnode(string):
	try:
		# Splits the string by spaces
		split_string = string.split();

		# Attempts to read each attribute of the string into FresNode parameters.
		# It is very important that when giving IDs to boards, we do not include any spaces. This will cause errors when parsing data. A library update may eventually be issued to prevent spaces from being used in the first place.
		id = split_string[0];

		timestamp = split_string[1];

		# Parses the timestamp into individual slices of time.
		year, month, day, hour, minute, second = parse_timestamp(timestamp);

		# Parses the data based on which datatype is read.
		if("Voltage" in string):
			datatype = "Voltage";
			data = parse_voltage(split_string);
		elif("Watermark" in string):
			datatype = "Watermark";
			data = parse_watermark(split_string);
		elif("Temperature" in string):
			datatype = "Temperature";
			data = parse_temperature(split_string);

		# If data readings are corrupt, report an error.
		if(data == None):
			print("Error parsing data from string: {!r}".format(string));
			return(None);

		# If all goes right, we create and return a new FresNode object from the parsed data!
		new_node = FresNode(id,year,month,day,hour,minute,second,datatype,data);
		return(new_node);

	except:
		print("Error parsing string into FresNode object: {!r}".format(string));
		return(None);

def parse_timestamp(timestamp):
	# Timestamp strings should be 'Year/Month/Day/Hour:Minute:Second'.
	try:
		# Replaces the '/' and ':' characters with ' ' for simpler splitting.
		timestamp.replace('/', ' ');
		timestamp.replace(':', ' ');
		split_time = timestamp.split();

		year = float(split_time[0]);
		month = float(split_time[1]);
		day = float(split_time[2]);
		hour = float(split_time[3]);
		minute = float(split_time[4]);
		second = float(split_time[5]);

		return(year, month, day, hour, minute, second);
	except:
		print("Error parsing timestamp into date/time data: {!r}".format(timestamp));
		# This will throw an exception in the parent function. This is intentional.
		return(None);

def parse_voltage(split_string):
	# Voltage strings should be 'ID timestamp "Battery-Voltage" battery_voltage "Solar-Voltage" solar_voltage'.
	try:
		data = [];

		battery_voltage = float(split_string[3]);
		solar_voltage = float(split_string[5]);
		data.append(battery_voltage);
		data.append(solar_voltage);
	except:
		print("Error parsing Voltage data: {!r}".format(split_string));
		return(None);

	return(data);

def parse_watermark(split_string):
	# Watermark strings should be 'ID timestamp "Watermark(s)" data data data...'.
	# The deployment on 7/28/2018 calls for six Watermark sensors per DAQ, but we'll generalize just to be safe.
	try:
		data = [];

		# Three is the starting 'index' of the Watermark data.
		for i in range(3,len(split_string)):
			substring = split_string[i];

			# If a Watermark sensor is not connected, the typical reading is "INF" or "NA".
			# Actually, if no Watermark sensors are connected, each port reads as "INF". If any sensors are connected, ports without sensors will read a large value (~90k) instead if infinity.
			# A future library update may fix this bug and switch "INF" readings to "NA".
			if(substring == "INF" or substring == "NA"):
				data.append("NA");
			else:
				data.append(float(substring))
	except:
		print("Error parsing Watermark data: {!r}".format(split_string));
		return(None);

	return(data);

def parse_temperature(split_string):
	# Temperature strings should be 'ID timestamp "Temperature" data data...'.
	# The deployment on 7/28/2018 calls for two Temperature sensors per DAQ, but we'll generalize just to be safe.
	try:
		data = [];

		# Three is the starting 'index' of the Temperature data.
		for i in range(3,len(split_string)):
			substring = split_string[i];

			# If a Temperature sensor is not connected, the typical reading is "-127". If for some reason the library updates to read an unconnected sensor as "NA", this should work fine.
			# Why would we change the library? In case someone sets up sensors in -127 degree F. weather! Doubtful but possible.
			if(substring == "NA" or substring == "-127"):
				data.append("NA");
			else:
				data.append(float(substring))
	except:
		print("Error parsing Temperature data: {!r}".format(split_string));
		return(None);

	return(data);

'''
