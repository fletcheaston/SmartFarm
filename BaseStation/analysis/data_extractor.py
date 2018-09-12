board_dict = {"B1":0, "B2":1, "F0":2, "F1":3, "FB":4, "F3":5, "F4":6, "F5":7, "F6":8, "FG":9};
b_data_dict = {"V":2, "W":3, "T":4};
f_data_dict = {"V":2, "W":6, "T":2};

relevant_data = [[] for x in range(len(board_dict))];

data_type = "T";

with open("all_data.txt") as f:
	line = f.readline();
	while(line):
		split_line = line.split()[2:];

		for i in range(len(split_line)):
			try:
				if(float(split_line[i]) > 80000):
					split_line[i] = "0";
			except:
				pass;

		if(data_type in split_line):
			relevant_data[board_dict[split_line[0]]].append(','.join(split_line[2:]));

		line = f.readline();

max = len(max(relevant_data,key=len));

with open("split_data.txt", "w") as f:
	for p in range(max):
		data_line = str(p) + ',';
		for i in range(len(relevant_data)):
			try:
				temp_line = relevant_data[i][p];
				data_line = data_line + temp_line + ',';
			except:
				if(i == 0 or i == 1):
					temp_line = "0," * b_data_dict[data_type];
				else:
					temp_line = "0," * f_data_dict[data_type];
				data_line = data_line + temp_line;

		data_line = data_line.replace("inf","0");
		data_line = data_line.replace("NA","0");
		data_line = data_line.replace("-127.00","0");
		data_line = data_line.replace("85.00","0");

		f.write(data_line);
		f.write("\n");
