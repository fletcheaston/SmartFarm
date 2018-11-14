#Created by Fletcher Easton

import data_logger;

# The F data is more compact than the B data, so the parsing is fairly different.
def reformat_and_log_f(data):
    data_list = data.split();

    if(len(data_list) == 1): # Fx with no decagon attached
        logged_data = data_list[0] + " D " + ' '.join(data_list[1:]);
        data_logger.log_raw_data(logged_data);

    elif(len(data_list) == 5): # Fx with decagon attached
        logged_data = data_list[0] + " D " + ' '.join(data_list[1:]);
        data_logger.log_raw_data(logged_data);

    elif(len(data_list) == 11):
        voltage = data_list[0] + " V " + ' '.join(data_list[1:3]); # Fx Battery_Voltage Solar_Voltage
        watermark = data_list[0] + " W " + ' '.join(data_list[3:9]); # Fx WM1 WM2 WM3 WM4 WM5 WM6
        if(data_list[0] == "FG"):
            temperature = data_list[0] + " T " + data_list[10] + " " + data_list[9]; # Fx Temp1 Temp2
        else:
            temperature = data_list[0] + " T " + data_list[9] + " " + data_list[10]; # Fx Temp1 Temp2
        data_logger.log_raw_data(voltage);
        data_logger.log_raw_data(watermark);
        data_logger.log_raw_data(temperature);

    else:
        data_logger.log_action("Error reformatting data: {!r}".format(data));
