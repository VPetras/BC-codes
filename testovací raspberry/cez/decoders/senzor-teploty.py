#!/usr/bin/env python3
import sys
import __future__

SIGFOX_HEADER_BOOT =  0x00
SIGFOX_HEADER_BEACON = 0x01
SIGFOX_HEADER_ALERT =  0x02

header_lut = {SIGFOX_HEADER_BOOT: 'BOOT', SIGFOX_HEADER_BEACON: 'BEACON', SIGFOX_HEADER_ALERT: 'ALERT'}


def temperature_decode(txt):
    if txt == 'ff':
        return None
    return int(txt, 16) / 2.0 - 28


def decode(data):
    if len(data) >= 6 and len(data) % 2 != 0:
        raise Exception("Bad data length 6 .. 24 and need modulo 2")

    header = int(data[0], 16)

    temperatures = {}

    temp_step = 20

    if header == SIGFOX_HEADER_ALERT:
        temp_step = 2

    length = int(((len(data) - 4) / 2))
    for i in range(length):
        c = 4 + i * 2
        t = -i * temp_step
        temperatures[t] = temperature_decode(data[c:c + 2])

    return  {
        "header": header_lut[header],
        "position": int(data[1], 16),
        "voltage": int(data[2], 16) / 8.0 + 1.8,
        "temperatures": temperatures,
    }


def pprint(data):
    print('Message :', data['header'])
    print('Position :', data['position'])
    print('Voltage :', data['voltage'])
    minuty = list(data['temperatures'].keys())
    minuty.sort(reverse=True)
    print('Temperatures:')
    for k in minuty:
        print('    ', k, 'min', data['temperatures'][k])



if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] in ('help', '-h', '--help'):
        print("usage: python3 decode.py [data]")
        print("example BOOT paket: python3 decode.py 067063")
        print("example BEACON paket: python3 decode.py 165063636363636363646567")
        print("example BEACON paket ALERT end: python3 decode.py 164066")
        print("example ALERT paket: python3 decode.py 26506b68676768686869696a")
        print("example ALERT packet for the first 20 minutes: python3 decode.py 26706a676666")
        exit(1)

    data = decode(sys.argv[1])
    pprint(data)
