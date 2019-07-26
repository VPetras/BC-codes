#!/usr/bin/env python3
import argparse
import os
import serial
import time
import subprocess
import sys
from bcf import flash_serial
from bcf.cli import print_progress_bar, download_url
from bcf.github_repos import Github_Repos
from relay_module import Relay_module
from collections import OrderedDict

def main():
    if os.geteuid() != 0:
        print("Run as root !!!! this command: sudo ./run.py")
        return False

    parser = argparse.ArgumentParser(description='USB Dongle tester')
    parser.add_argument('--device', help='device', default="/dev/ttyUSB0")
    parser.add_argument('--jump-ftdi-pwren', action='store_true')
    parser.add_argument('--test-firmware-bin',  default="firmware-sensation-2-2-test-fw.bin")
    
    args = parser.parse_args()

    relay = Relay_module()
    relay.state(True)

    if not args.jump_ftdi_pwren:

        print("Zapisuju PWREN do ftdi")
        is_rpi = os.uname()[-1] == 'armv7l'

        cmd = ["./ftx_prog_rpi" if is_rpi else "./ftx_prog", "--cbus", "3", "PWREN"]

        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        if "CBUS3 = PWREN" not in str(stdout):
            print("Nepovedl se zapis PWREN")
            return False

        relay.state(False)

        time.sleep(1)

        relay.state(True)

        time.sleep(2)
        
        while not os.path.exists(args.device):
            print("Odpojit a pripojit sensatio prosim")
            time.sleep(2)

    if args.test_firmware_bin != "jump":

        print("=== Flash %s ===" % args.test_firmware_bin)

        flash_serial.run(args.device, args.test_firmware_bin, reporthook=print_progress_bar)

        time.sleep(0.5)

    print("=== Test ===")

    sumarize = OrderedDict()
    sumarize['WSSFM10R1AT: Device ID'] = None
    sumarize['WSSFM10R1AT: Device PAC'] = None
    sumarize['SHT30: Humidity'] = None
    sumarize['SHT30: Temperature'] = None
    sumarize['CO2: Concentration'] = None
    sumarize['OPT3001: Illuminance'] = None
    sumarize['MPL3115A2: Pressure'] = None
    sumarize['PYQ1648'] = None
    sumarize['Battery: ADC value'] = None
    sumarize['MIC: Noise level'] = None

    ser = serial.Serial(args.device, baudrate=115200, timeout=3.0)

    while True:
        try:
            try:
                line = ser.readline()
            except Exception as e:
                print("CHYBA seriovy port")
                ser.close()
                return False
            
            if line:
                line = line.decode().rstrip()
                
                print(line)

                line = line[line.find('>') + 2:]

                if "=" in line:
                    key, value = line.split("=", 1)
                    
                    key = key.strip()
                    if not key in sumarize:
                        print('neznamy key', key)
                        # return False

                    sumarize[key] = value.strip()

                elif "PYQ1648" in line:
                    sumarize['PYQ1648'] = line.split(':')[1].strip()
                
                elif "Error" in line:
                    line = line.split(":", 1)
                    for key in sumarize:
                        if key.startswith(line[0]):
                            sumarize[key] = line[1].strip()

        except KeyboardInterrupt as e :
            print("")
            for key, value in sumarize.items():
                print(key, '=', value)
            break
    
    ser.close()

if __name__ == '__main__':
    sys.exit(0 if main() else 1)

