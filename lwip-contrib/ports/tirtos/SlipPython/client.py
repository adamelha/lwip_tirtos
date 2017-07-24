#!/usr/bin/env python
import socket
import binascii
import threading
import sys

#from scapy import *
# SLIP decoder

DEVICE_IP = "192.168.1.17"
DEVICE_PORT = 10000

CLIENT_PORT = 50000
COMMANDS = ["TEMP", "BAT", "NOT A COMMAND"]
COMMAND_DESC = {'TEMP' : 'Gets device room temperature in celsius',
                'BAT' : 'Gets device battery in volts',
                'NOT A COMMAND' : ' This API does not exist'}

def send_udp_packet():
    global DEVICE_IP, DEVICE_PORT, COMMANDS, COMMAND_DESC

    # Create and send UDP socket with desired command
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(5)

    try:
        for cmd in COMMANDS:
            sock.sendto(cmd, (DEVICE_IP, DEVICE_PORT))
            print 'Sent command {} : {}'.format(cmd, COMMAND_DESC[cmd])
            data, sender_addr = sock.recvfrom(1024)
            print 'data from {}: {}'.format(sender_addr, data)

        sock.close()

    except:
        print 'Exception caught'
        sock.close()
        raise
        sys.exit()


def main():
    try:
        thread = threading.Thread(target=send_udp_packet)
        thread.start()

        while True:
            pass
    except:
        sys.exit()

if __name__ == '__main__':
    main()