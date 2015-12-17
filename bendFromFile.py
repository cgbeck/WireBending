import sys
import os.path
import numpy as np

if len(sys.argv) == 2:
    filename = sys.argv[1]
    if os.path.isfile(filename):

        f = open(filename, 'r')

        # get data from specified file
        # data = np.genfromtxt(filename, delimiter=' ', skip_header=0, names=['action', 'amount'])

        # start reading from serial
        ser = serial.Serial('/dev/cu.usbmodem1421', 9600)

        print f.read()
        ser.write('5')

        # process data
        # time = np.array(data['time'])
        # leftM = np.array(data['leftM'])
        # rightM = np.array(data['rightM'])
        # leftS = np.array(data['leftS'])
        # rightS = np.array(data['rightS'])
    else:
        print 'Specified file does not exist'
else:
    print 'No file specified'