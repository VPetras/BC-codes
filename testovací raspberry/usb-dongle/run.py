#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import httplib2
import os
import serial
import time
import subprocess
import sys
from bcf.flasher import flash
from bcf.cli import print_progress_bar, download_url, user_cache_dir, user_config_dir
from bcf.github_repos import Github_Repos
from PIL import Image, ImageDraw, ImageFont
from apiclient import discovery
from oauth2client import client
from oauth2client import tools
from oauth2client.file import Storage
from colorama import init, Fore, Style

APPLICATION_NAME = 'USB Dongle tester'
IS_RPI = os.uname()[-1] == 'armv7l'


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


def run(device, usb_dongle_firmware):

    if not os.path.exists(device):
        print("Pripojit dongle prosim")

        while not os.path.exists(device):
            time.sleep(2)

        time.sleep(2)

    print(Fore.YELLOW + "=== Flash bcf-tester-usb-dongle.bin ===")

    flash("bcf-tester-usb-dongle.bin", device, reporthook=print_progress_bar)

    time.sleep(0.5)

    print(Fore.YELLOW + "=== Test ===")

    ser = serial.Serial(device, baudrate=115200, timeout=3.0)

    atsha204_i2c0_id = None
    atsha204_i2c1_id = None

    ok_texty = {}

    while True:
        try:
            line = ser.readline()
        except Exception as e:
            try:
                ser.close()
            except Exception as e:
                pass
            raise Exception("CHYBA seriovy port: " + str(e))

        line = line.strip()

        if line:
            line = line.decode()
            if line.startswith("# <E> "):
                raise Exception("CHYBA " + str(line))

            if not line.startswith("# <I> "):
                continue

            line = line[6:]

            sline = line.split(" ", 2)
            text = sline[0] + " " + sline[1]
            if text not in ok_texty:
                print(text, 'OK')
                ok_texty[text] = sline[2]

            if len(ok_texty) == 4:
                break

    ser.close()

    if not ok_texty['atsha204 i2c0']:
        raise Exception("Empty atsha204_i2c0_id")

    print(Fore.YELLOW + "=== Write iSerial to FTDI ===")

    iSerial = 'bc-usb-dongle-r1.0-' + ok_texty['atsha204 i2c0']

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

    # while not os.path.exists(device):
    #     print("Odpojit a pripojit dongle prosim")
    #     time.sleep(2)

    # time.sleep(2)

    # print(Fore.YELLOW + "=== Flash bcf-gateway-usb-dongle ===")

    # print('name', usb_dongle_firmware['name'])

    # usb_dongle_firmware_bin = download_url(usb_dongle_firmware['download_url'])

    # flash(usb_dongle_firmware_bin, device, reporthook=print_progress_bar, use_dfu=False, run=True)

    return {"atsha204_sn": node_id_to_sn(ok_texty['atsha204 i2c0']),
            "atsha204_i2c1": node_id_to_sn(ok_texty['atsha204 i2c1'])}


product_name_lut = {'bc-usb-dongle-r1-': 'USB Dongle R1.0'}

def main():
    spreadsheet_id = '1fuPB4bwQ8_RaiHlOcM8E4XiPLnKGJNKt6omWwRMIUsE'

    parser = argparse.ArgumentParser(description=APPLICATION_NAME)
    parser.add_argument('--device', help='device', default="/dev/ttyUSB0")
    parser.add_argument('--prefix', default='bc-usb-dongle-r1-')
    parser.add_argument('--firmware', default='bigclownlabs/bcf-gateway-usb-dongle:latest')
    parser.add_argument('--noauth_local_webserver', help=argparse.SUPPRESS, action='store_true', default=True)
    parser.add_argument('--logging_level', default='ERROR', help=argparse.SUPPRESS)

    args = parser.parse_args()

    product_name = product_name_lut[args.prefix]

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

    #img = Image.new('L', (200, 100), 255)
    #font = ImageFont.truetype("arial.ttf", 28)
    #d = ImageDraw.Draw(img)
    #d.text((20, 20), 'Hello', 0, font)
    #img.save("test.bmp", 'bmp')
    #return

    usb_dongle_firmware = None

    # repos = Github_Repos(user_config_dir, user_cache_dir)

    # usb_dongle_firmware = repos.get_firmware(args.firmware)

    # if not usb_dongle_firmware:
    #     print('Firmware not found, try updating first')
    #     print('command: bcf update')
    #     sys.exit(1)

    while True:

        info = run(args.device, usb_dongle_firmware)

        if info["atsha204_sn"] not in sha204a_sn_google:
            print(Fore.YELLOW + "=== Add serial numbers to google sheets ===")
            print("atsha204 sn", info["atsha204_sn"])
            values = [['=(((INDIRECT(CONCAT("D", ROW()))/60)/60)/24)+DATE(1970,1,1)', product_name, "", str(int(time.time())), "ff:ff", "ff:ff", info["atsha204_sn"], "00:04:09:00", info["atsha204_i2c1"]]]
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

        print(Fore.GREEN + "=== Hotovo ===")
        print("Nyní lze odpojit dongle")
        while os.path.exists(args.device):
            time.sleep(2)


if __name__ == '__main__':

    init(autoreset=True)

    try:
        main()
    except KeyboardInterrupt as e:
        sys.exit()
    except Exception as e:
        print(Fore.RED + str(e))
        sys.exit(1)
