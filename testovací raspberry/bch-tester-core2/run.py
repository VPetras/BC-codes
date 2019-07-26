#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import httplib2
import os
import serial
import time
import subprocess
import sys
from bcf.flasher import flash, reset
from bcf.repos.github import Github as Github_Repos
from bcf.cli import print_progress_bar, download_url, user_cache_dir, user_config_dir
from apiclient import discovery
from oauth2client import client
from oauth2client import tools
from oauth2client.file import Storage
from colorama import init, Fore, Style
import json

APPLICATION_NAME = 'CORE MODULE R2 tester'
IS_RPI = os.uname()[-1] == 'armv7l'

TEST_FIRMWARE_BIN = 'bcf-tester-core2-core.bin'


if IS_RPI:
    user_cache_dir = '/home/pi/.cache/bcf'
    user_config_dir = '/home/pi/.config/bcf'


def google_get_credentials(args):
    if not os.path.exists(user_config_dir):
        os.makedirs(user_config_dir)
    credential_path = os.path.join(user_config_dir, 'sheets.googleapis.com-python-quickstart.json')

    store = Storage(credential_path)
    credentials = store.get()
    if not credentials or credentials.invalid:
        flow = client.flow_from_clientsecrets('client_secret.json', 'https://www.googleapis.com/auth/spreadsheets')
        flow.user_agent = APPLICATION_NAME
        credentials = tools.run_flow(flow, store, flags=args)
        print('Storing credentials to ' + credential_path)
    return credentials


def node_id_to_sn(node_id):
    if len(node_id) != 12:
        raise Exception('Bad node id length')

    tmp = ['ee']
    for i in range(0, 12, 2):
        tmp.append(node_id[i] + node_id[i + 1])
    tmp.append('23')
    tmp.append('01')
    return ':'.join(tmp)


def run(port_tester, port_device, prefix, no_radio):

    if not os.path.exists(port_tester):
        raise Exception("Neni pripojen tester")

    if not os.path.exists(port_device):
        print("Pripojit core prosim")

        while not os.path.exists(port_device):
            time.sleep(0.1)

        time.sleep(1)

    print(Fore.YELLOW + "=== Flash " + TEST_FIRMWARE_BIN + " ===")

    i = 0
    while True:
        try:
            flash(TEST_FIRMWARE_BIN, port_device, reporthook=print_progress_bar)
            break
        except Exception as e:
            i += 1
            if i == 4 :
                raise
            time.sleep(0.5)

    reset(port_device)

    print(Fore.YELLOW + "=== Test ===")

    tester = serial.Serial(port_tester, timeout=3.0)

    device = serial.Serial(port_device, baudrate=115200, timeout=3.0)

    atsha204_i2c0_id = None

    wait_for = {
        'application_init end': False,
        'atsha204 update': False,
        'lis2dh12 update': False,
        'lis2dh12 int': False,
        'tmp112 update': False,
        'tmp112_internal update': False
        }

    if not no_radio:
        wait_for['spirit1 tx ok'] = False
        wait_for['spirit1 rx ok'] = False

    tester.write(b'\n')

    pin = 0

    pin_test = False
    pin_test_timeout = 0

    def tester_wait_confirm(meesage):
        timeout = time.time() + 5
        while True:
            line = tester.readline()

            if line:
                if line == meesage:
                    return line

                print('tester', line)

            if time.time() > timeout:
                raise Exception('tester timeout')


    tester.write(json.dumps(['/gpio/reset', None]).encode() + b'\n')
    tester_wait_confirm(b'["gpio/reset/ok", null]\n')

    start_time = time.time()

    def gpio_set(pin, state):
        tester.write(json.dumps(['/gpio/set', {'pin': pin, 'state': state}]).encode() + b'\n')

    while True:
        try:
            line = device.readline()
        except KeyboardInterrupt as e:
            sys.exit()
        except Exception as e:
            try:
                device.close()
            except Exception as e:
                pass
            raise Exception("CHYBA seriovy port: " + str(e))

        line = line.strip()

        if line:
            line = line.decode()
            if line.startswith("# <E> "):
                if no_radio:
                    if line == '# <E> bc_spirit1_init':
                        continue
                print( Fore.RED + "CHYBA " + str(line) )
                if input("pokud chcete pokracovat napiste next a enter, jinak ukoncit: ").strip() != "next":
                    raise Exception("CHYBA " + str(line))

            if not line.startswith("# <I> "):
                continue

            line = line[6:]

            if line.startswith("atsha204"):
                atsha204_i2c0_id = line.split()[1]
                line = 'atsha204 update'

            if pin_test:
                # print('pin_test:', line)
                if time.time() > pin_test_timeout:
                    print( Fore.RED + 'gpio pin p%d' % pin)

                if line == 'gpio pin p%d' % pin:
                    gpio_set(pin, False)
                    pin += 1
                    if pin == 10:
                        pin = 12

                    if pin > 18:
                        print('piny otestovany')
                        break

                    gpio_set(pin, True)

            elif line in wait_for:
                if not wait_for[line]:
                    wait_for[line] = True
                    print(line)

                if line == 'atsha204 update':
                    tester.write(json.dumps(['/spirit/tx', None]).encode() + b'\n')

                for k in wait_for:
                    if not wait_for[k]:
                        if time.time() > (start_time + 10):
                            if k == 'spirit1 rx ok':
                                tester.write(json.dumps(['/spirit/tx', None]).encode() + b'\n')
                            print('wait on', k)
                        break
                else:
                    if not pin_test:
                        print("Run pin test")
                        pin_test = True
                        pin_test_timeout = time.time() + 1
                        gpio_set(pin, True)

            elif line == "application_init run":
                pass
            else:
                print('line:', line)

    device.close()

    print('atsha204_i2c0_id', atsha204_i2c0_id)

    print(Fore.YELLOW + "=== Write iSerial to FTDI ===")

    iSerial = prefix + atsha204_i2c0_id

    print("iSerial", iSerial)

    cmd = ["./ftx_prog_rpi" if IS_RPI else "./ftx_prog", "--new-serial-number", iSerial, "--manufacturer", "", "--product", ""]

    if os.getenv('DEBUG', False):
        print('cmd', cmd)

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    if iSerial not in str(stdout):
        if os.getenv('DEBUG', False):
            print('stdout', stdout)
        raise Exception("CHYBA zapisu ftx_prog")

    # kontrola
    p = subprocess.Popen(["lsusb", "-d", "0403:6015", "-v"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    if iSerial not in  str(stdout):
        raise Exception("CHYBA zapisu lsusb")

    return {"atsha204_sn": node_id_to_sn(atsha204_i2c0_id), 'atsha204_i2c0_id': atsha204_i2c0_id, 'iSerial': iSerial }

product_name_lut = {'bc-core-module-r2-': 'Core Module R2.0'}

def main():
    spreadsheet_id = '1fuPB4bwQ8_RaiHlOcM8E4XiPLnKGJNKt6omWwRMIUsE'

    parser = argparse.ArgumentParser(description=APPLICATION_NAME)
    parser.add_argument('--tester', help='tester', default="/dev/ttyACM0")
    parser.add_argument('--device', help='device', default="/dev/ttyUSB0")
    parser.add_argument('--no-radio', action='store_true')

    parser.add_argument('--prefix', default='bc-core-module-r2-')
    parser.add_argument('--firmware')

    parser.add_argument('--noauth_local_webserver', help=argparse.SUPPRESS, action='store_true', default=True)
    parser.add_argument('--logging_level', default='ERROR', help=argparse.SUPPRESS)

    args = parser.parse_args()

    product_name = product_name_lut[args.prefix]

    if not os.path.exists(args.tester):
        print(Fore.RED + "Neni pripojen tester")
        sys.exit(1)

    if args.firmware:
        if not 'radio' in args.firmware:
            print('radio not in firmware name')
            sys.exit(1)

        repos = Github_Repos(user_config_dir, user_cache_dir)
        firmware = repos.get_firmware(args.firmware)
        if not firmware:
            print('Firmware not found, try updating first, use command: bcf update')
            sys.exit(1)
        filename_bin = download_url(firmware['download_url'])

    credentials = google_get_credentials(args)
    http = credentials.authorize(httplib2.Http())
    discoveryUrl = ('https://sheets.googleapis.com/$discovery/rest?version=v4')
    service = discovery.build('sheets', 'v4', http=http, discoveryServiceUrl=discoveryUrl)

    print(Fore.YELLOW + "=== Load serial numbers from google ===")
    result = service.spreadsheets().values().get(spreadsheetId=spreadsheet_id, range="G:G").execute()

    if not result:
        raise Exception('Google spreadsheet empty result')

    if result['values'].pop(0)[0] != 'SHA204A SN':
        raise Exception('Google spreadsheet bad column')

    sha204a_sn_google = {row[0] for row in result['values']}

    while True:

        info = run(args.tester, args.device, args.prefix, args.no_radio)

        # print(info)

        if info["atsha204_sn"] not in sha204a_sn_google:
            print(Fore.YELLOW + "=== Add serial numbers to google sheets ===")
            print("atsha204 sn", info["atsha204_sn"])
            values = [['=(((INDIRECT(CONCAT("D", ROW()))/60)/60)/24)+DATE(1970,1,1)', product_name, "", str(int(time.time())), "ff:ff", "ff:ff", info["atsha204_sn"], "00:04:09:00" ]]
            body = {
                'values': values
            }
            result = service.spreadsheets().values().append(
                        spreadsheetId=spreadsheet_id,
                        range="A1",
                        valueInputOption="USER_ENTERED",
                        body=body
            ).execute()
            if os.getenv('DEBUG', False):
                print('result', result)
            if result['updates']['updatedRows'] != 1:
                raise Exception('google sheets append error')
            sha204a_sn_google.update([info["atsha204_sn"]])
        else:
            print("Seriove cislo v databazi jiz existuje")

        print(Fore.LIGHTYELLOW_EX + "Odpojit core z USB a stisk TLACITKA a na multimetru max 3uA")

        if args.firmware:

            time.sleep(3)

            print("Nyní připojit core pro flesovani", args.firmware)

            while not os.path.exists(args.device):
                time.sleep(0.1)

            time.sleep(1)

            i = 0
            while True:
                try:
                    flash(filename_bin, args.device, reporthook=print_progress_bar)
                    break
                except Exception as e:
                    i += 1
                    if i == 4 :
                        raise
                    time.sleep(0.5)

            print(Fore.GREEN + "=== Hotovo ===")

            print("Nyní lze odpojit core")

        while os.path.exists(args.device):
            time.sleep(0.1)


if __name__ == '__main__':
    init(autoreset=True)

    try:
        main()
    except KeyboardInterrupt as e:
        sys.exit()
    except Exception as e:
        print(Fore.RED + str(e))
        sys.exit(1)
