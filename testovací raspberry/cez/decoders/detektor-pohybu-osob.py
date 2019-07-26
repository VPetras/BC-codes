#!/usr/bin/env python3
import sys
import __future__

SIGFOX_HEADER_BOOT =  0x00
SIGFOX_HEADER_BEACON = 0x01
SIGFOX_HEADER_ALERT =  0x02

header_lut = {SIGFOX_HEADER_BOOT: 'BOOT', SIGFOX_HEADER_BEACON: 'BEACON', SIGFOX_HEADER_ALERT: 'ALERT'}


def decode(data):
    if len(data) != 6 and len(data) != 10:
        raise Exception("Bad data length, 6 or 10 characters expected")

    header = int(data[0], 16)

    return {
        "header": header_lut[header],
        "position": int(data[1], 16),
        "voltage": int(data[2], 16) / 8.0 + 1.8,
        "temperature": int(data[4:6], 16) / 2.0 - 28,
        "count": int(data[6:10], 16) if header == SIGFOX_HEADER_ALERT else None
    }


def pprint(data):
    print('Message :', data['header'])
    print('Position :', data['position'])
    print('Voltage :', data['voltage'])
    print('Temperature :', data['temperature'])
    if data['count'] is not None:
        print('Event count :', data['count'])


if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] in ('help', '-h', '--help'):
        print("usage: python3 decode.py [data]")
        print("example BOOT paket: python3 decode.py 06706b")
        print("example BEACON paket: python3 decode.py 168064")
        print("example ALERT paket: python3 decode.py 23706b0001")
        print("example ALERT paket: python3 decode.py 2250640000")
        exit(1)

    data = decode(sys.argv[1])
    pprint(data)
