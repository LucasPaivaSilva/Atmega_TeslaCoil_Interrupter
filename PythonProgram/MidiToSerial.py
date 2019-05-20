"""
Receive messages from the input port and print them out.
"""
from __future__ import print_function
import sys
import mido
data = ''

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
            if message.velocity == 0:
            	data = '1'
            else:
            	data = '0'
            data = data + str(message.note)
            print(data)
            sys.stdout.flush()
except KeyboardInterrupt:
    pass