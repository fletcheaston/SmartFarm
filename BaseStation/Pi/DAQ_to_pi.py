'''
Wiritten By: Caleb Fink
Created: ~5/10/17
Last Updated: 11/27/17
'''

#!/usr/bin/env python
# coding: utf8

#SmartFarm_Parser.py
#daq2rpi_parser.py


import serial
import os
from time import sleep
import datetime

# this log keeps track of which script is running
runlog_file = "daq2rpi_runlog.txt"
# this script logs the parsed DAQ data
datalog_filepath = "/var/log/smartfarm/SmartFarmData.log"
# keep a log of when the data was written on the basestation
BaseStationlog_file = "BaseStation_Log.txt"

#Changed UPLOAD_Log to daq2rpi_log
file = open(runlog_file,"a")
Timestamp ='{:%Y-%b-%d %H:%M:%S}'.format(datetime.datetime.now())
log = Timestamp+" [daq2rpi_parser.py]>Called and running...\n"
file.write(log)
print log
file.close()
#Type ls /dev/tty* to find the com port, Connect, then disconnect to find the appropriate one.
#an auto USB Xbee finder would be great here in case it gets disconnected
serialPort = '/dev/ttyUSB0'
#serialPort = '/dev/ttyACM0'
serialBaudRate = 57600
SerialData = serial.Serial(serialPort,serialBaudRate)
sleep(2) # wait for serial connection
SerialData.flushInput()
SerialData.flushOutput()
letters = []
word = []
data = []
trigger = []
BoardIdentifiers = ["B1","B2"]
tmpDataIndexes = []
tmpData = []
#Sample String of data
''' B1 Upload B2 Upload B1 110_0:06:34 B2 103017_10:10:39 B1 3.97 0.00 B2 3.82 0.00 B2 1986.098 5485.205 9984.210 B1 9837.750 5337.930 1875.300  B2 NA NA NA NA B2 55.60 55.78 54.20 54.23B2 B1 NA NA NA B1
#needs parsing to

#B1 Upload B1 110_0:06:34 B1 3.97 0.00 B1 9837.750 5337.930 1875.300  B1 9837.750 5337.930 1875.300  B1 NA NA NA B1
#B2 Upload B2 103017_10:10:39 B2 3.82 0.00 B2 1986.098 5485.205 9984.210 B2 NA NA NA NA B2 55.60 55.78 54.20 54.23B2
#in a separate file '''
try:
    while True:
    # the loop reads in characters
        # while serial data is available
        #or inWaiting()
        while SerialData.in_waiting:
            #store the serial data as an array characters.
#letters = 'B''1'' ''U''p''l''o''a''d'' ''B''2'' ''U''p''l''o''a''d'' ''B''1'' ''1''1''0''_''0'':''0''6'':''3''4'' ''B''2'' ''1''0''3''0''1''7''_''1''0'':''1''0'':''3''9'' ''B''1'' ''3''.''9''7'' '...''0''0'' '' ''B''2'' ''N''A'' ''N''A'' ''N''A'' ''N''A'' ''B''2'' ''5''5''.''6''0'' ''5''5''.''7''8'' ''5''4''.''2''0'' ''5''4''.''2''3''B''2'' ''B''1'' ''N''A'' ''N''A'' ''N''A'' ''B''1''
            letters += SerialData.read()
            #combine the characters into word arrays as the letters come in
            word = ''.join(letters)
            tmpData = letters[1:len(letters)] # ignore the first B
            for i, j in enumerate(tmpData):
                if j == 'B':
                    #tmpDataIndexes +=i
                    tmpData = letters
                    data = 'B'    # a preceding 'B', this gets dropped somehow in the string
                    #join the letters together and write them to data with a 'B' preceding it.
                    data += ''.join(tmpData[0:i])
                    if "BB" in data:
                            data = data[1:len(data)]
                    # The data log where daq data is stored
                    file = open(datalog_filepath,"a")
                    log = data #data to be written
                    file.write(log)
                    print log
                    file.close()
                    # keep a log of when the data was written on the basestation
                    file = open(BaseStationlog_file,"a")
                    Timestamp ='{:%Y-%b-%d %H:%M:%S}'.format(datetime.datetime.now())
                    log = Timestamp+" [daq2rpi_parser.py]>Called and running...\n"
                    file.write(log)
                    #print log
                    file.close()
                    #Clear the data for a new set
                    letters = []
                    tmpData = []
                    word = []
                    data = []
# Does this work with multiple boards??? Actually, I think it will, but it doesn't separate,
#it basically just replicates what it gets
# this script needs help
#

except KeyboardInterrupt:
    SerialData.flushInput()
    SerialData.flushOutput()
    file = open(runlog_file,"a")
    log = "[daq2rpi_parser.py]>Script Canceled\n"
    file.write(log)
    print log
    file.close()
    os._exit(-1)
