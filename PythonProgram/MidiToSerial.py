"""
Receive messages from the input port and print them out.
"""
from __future__ import print_function
import sys
import mido
import serial
import datetime
data = ''

ser = serial.Serial('COM5', 9600)
ser.timeout = 0.01
if len(sys.argv) > 1:
    portname = sys.argv[1]
else:
    portname = None  # Use default port

try:
    with mido.open_input(portname) as port:
        print('Using {}'.format(port))
        print('Waiting for messages...')
        for message in port:
            print('Received {}'.format(message))
            if message.type == 'note_on':
            	data = 'L'
            else:
            	data = 'D'
            data = data + str(message.note)
            print(data)
            ser.write(data.encode())
            sys.stdout.flush()
except KeyboardInterrupt:
    pass
