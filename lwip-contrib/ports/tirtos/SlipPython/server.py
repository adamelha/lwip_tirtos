#!/usr/bin/env python
import serial
import binascii
import socket
import threading
import codecs
from scapy.all import *

DEVICE_IP = "192.168.1.17"
DEVICE_PORT = 10000

ser = None
class slip():
    def __init__(self):
        self.started = False
        self.escaped = False
        self.stream = ''
        self.packet = ''
        self.SLIP_END = '\xc0'  # dec: 192
        self.SLIP_ESC = '\xdb'  # dec: 219
        self.SLIP_ESC_END = '\xdc'  # dec: 220
        self.SLIP_ESC_ESC = '\xdd'  # dec: 221
        self.serialComm = None
        self.test = ''

    def append(self, chunk):
        self.stream += chunk
    def decode(self):
        packetlist = []
        for char in self.stream:
            # SLIP_END
            if char == self.SLIP_END:
                if self.started:
                    packetlist.append(self.packet)
                else:
                    self.started = True
                self.packet = ''
            # SLIP_ESC
            elif char == self.SLIP_ESC:
                self.escaped = True
            # SLIP_ESC_END
            elif char == self.SLIP_ESC_END:
                if self.escaped:
                    self.packet += self.SLIP_END
                    self.escaped = False
                else:
                    self.packet += char
            # SLIP_ESC_ESC
            elif char == self.SLIP_ESC_ESC:
                if self.escaped:
                    self.packet += self.SLIP_ESC
                    self.escaped = False
                else:
                    self.packet += char
            # all others
            else:
                if self.escaped:
                    raise Exception('SLIP Protocol Error')
                    self.packet = ''
                    self.escaped = False
                else:
                    self.packet += char
                    self.started = True
        self.stream = '' #// FIXME: should not be commented
        self.started = False
        return (packetlist)

    def encode(self, packet):
        # Encode an initial END character to flush out any data that
        # may have accumulated in the receiver due to line noise
        encoded = self.SLIP_END
        for char in packet:
            # SLIP_END
            if char == self.SLIP_END:
                encoded += self.SLIP_ESC + self.SLIP_ESC_END
            # SLIP_ESC
            elif char == self.SLIP_ESC:
                encoded += self.SLIP_ESC + self.SLIP_ESC_ESC
            # the rest can simply be appended
            else:
                encoded += char
        encoded += self.SLIP_END
        return (encoded)

slip_obj = None

# Reads serial line to receive answer from device and sends to client

def receive_from_device():
    global ser
    received = ''
    while True:
        slip_read = slip()
        received += ser.read(1)

        slip_read.append(received)
        response = slip_read.decode()
        if (len(response) > 0):
            ip_packet = IP(response[len(response) - 1])

            # Reset decoder
            slip_read.stream = ''
            received = ''
            if ip_packet is not None:
                if (ip_packet[IP].sport == 10000):
                    send(ip_packet)


def process_packet_from_client(pkt):
    global ser
    if IP in pkt:
        print 'Received IP packet from client:'
        pkt[IP].show()

        # Send only packets that have UDP header to device.
        # Not necessary but makes it easier on device
        if 'UDP' in pkt:
            slip_obj = slip()
            slip_packet = slip_obj.encode(str(pkt[IP]))
            print "slip packet to be sent to Device: {}".format(slip_packet)

            ser.write(slip_packet)

# sniffs packets from clients and forward to device
def sniff_from_client():
    global DEVICE_IP, DEVICE_PORT, ser
    a = sniff(filter="host {}".format(DEVICE_IP), prn=process_packet_from_client, timeout=60)

def main():
    global slip_obj, DEVICE_IP, DEVICE_PORT, COMMAND, ser

    try:
        ser = serial.Serial(port="COM3", baudrate=115200, stopbits=serial.STOPBITS_ONE)

        # Sniff packets from clients and forward to device
        thread_process_client_packets = threading.Thread(target=sniff_from_client)
        thread_process_client_packets.start()

        # Read serial line to receive answer from device and sends to client
        thread_process_device_packets = threading.Thread(target=receive_from_device)
        thread_process_device_packets.start()

        while True:
            pass

    except:
        ser.close()
        print "not connected"

if __name__ == '__main__':
    main()