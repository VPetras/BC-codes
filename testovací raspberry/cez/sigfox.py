#!/usr/bin/env python3

import argparse
import re
import os
import sys
import urllib.parse
import requests
import imp
import datetime
import json
import csv

DECODERS_PATH = 'decoders'
SIGFOX_API_LOGIN = '5a60bcb69e93a1755c4d03d1'
SIGFOX_API_PASSWORD = 'a1b26b18293c169a758e0723e78ffa36'

decoders = {}

for filename in os.listdir(DECODERS_PATH):
    name, ext = os.path.splitext(filename)
    filename = os.path.join(DECODERS_PATH, filename)
    if os.path.isfile(filename) and ext == ".py":
        decoders[name] = filename

choices_decoder = tuple(decoders.keys())
choices_output = ('print', 'json', 'csv')

parser = argparse.ArgumentParser()
parser.add_argument('decoder', metavar="DECODER", help='mecoder values ' + str(choices_decoder), choices=choices_decoder)
parser.add_argument("device_id", metavar="DEVICE-ID", help="sigfox device id")
parser.add_argument('--limit', help='max messages (default=1)', type=int, metavar='N', default=1)
parser.add_argument('--output', metavar='OUTPUT', help='output values ' + str(choices_output) + ' (default=print)', choices=choices_output, default='print')
args = parser.parse_args()

r = requests.get('https://backend.sigfox.com/api/devices/%s/messages?limit=%s' % (args.device_id, args.limit), auth=(SIGFOX_API_LOGIN, SIGFOX_API_PASSWORD))
json_data = r.json()

decoder = imp.load_source(args.decoder, decoders[args.decoder])

all_data = []

if args.output == 'csv':
    spamwriter = csv.writer(sys.stdout)
    first = True

for i, row in enumerate(json_data['data']):
    data = decoder.decode(row['data'])
    dt = datetime.datetime.fromtimestamp(row['time'])
    if args.output == 'print':
        print(dt, '\t\tdata:', row['data'])
        decoder.pprint(data)
        print("*" * 60)
    elif args.output == 'csv':
        if first:
            first = False
            spamwriter.writerow(['time', 'dt', 'data'] + list(data.keys()))
        spamwriter.writerow([row['time'], dt.isoformat(), row['data']] + list(data.values()))

    else:
        data['_time'] = row['time']
        data['_data'] = row['data']
        data['_dt'] = dt.isoformat()
        all_data.append(data)

if args.output == 'json':
    print(json.dumps(all_data, sort_keys=True, indent=2))

