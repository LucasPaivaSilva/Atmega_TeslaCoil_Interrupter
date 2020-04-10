"""
Receive messages from the input port and print them out.
"""
from __future__ import print_function
import sys
import mido
import serial
import datetime
data = ''

ser = serial.Serial('/dev/tty.usbmodem1491', 9600)
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
            print(message)
            print(message.type)
            if (message.type != 'control_change') and (message.type != 'pitchwheel') and (message.type != 'program_change'):   
                if message.type == 'note_on':
                    data = 'L'
                else:
                    data = 'D'
                if message.note <= 16:
                    message.note = 16
                if message.note >= 70:
                    message.note = 70
                data = data + str(message.note)
                print(data)
                print('')
            ser.write(data.encode())
            sys.stdout.flush()
except KeyboardInterrupt:
    pass
