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

def main():
    if os.geteuid() != 0:
        print("Run as root !!!! this command: sudo ./run.py")
        return False

    parser = argparse.ArgumentParser(description='USB Dongle tester')
    parser.add_argument('--device', help='device', default="/dev/ttyUSB0")

    args = parser.parse_args()

    # relay = Relay_module()
    # relay.state(True)

    # print("Zapisuju PWREN do ftdi")
    # is_rpi = os.uname()[-1] == 'armv7l'

    # cmd = ["./ftx_prog_rpi" if is_rpi else "./ftx_prog", "--cbus", "3", "PWREN"]

    # p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # stdout, stderr = p.communicate()
    # if "CBUS3 = PWREN" not in str(stdout):
    #     print("Nepovedl se zapis PWREN")
    #     return False

    # relay.state(False)

    # time.sleep(0.5)

    # relay.state(True)

    # time.sleep(0.5)
    
    # while not os.path.exists(args.device):
    #     print("Odpojit a pripojit sensatio prosim")
    #     time.sleep(2)

    # time.sleep(2)

    print("=== Flash firmware-sensation-2-2-test-fw.bin ===")

    flash_serial.run(args.device, "firmware-sensation-2-2-test-fw.bin", reporthook=print_progress_bar)

    time.sleep(0.5)

    print("=== Test ===")

    ser = serial.Serial(args.device, baudrate=115200, timeout=3.0)

    sumarize = {}

    while True:
        try:
            line = ser.readline()
        except Exception as e:
            print("CHYBA seriovy port")
            ser.close()
            return False
        
        if line:
            line = line.decode().rstrip()
            line = line[line.find('>') + 2:]

            if "=" in line:
                key, value = line.split("=", 1)
                sumarize[key] = value

            elif "PYQ1648" in line:
                sumarize[line] = None
            elif "Error" in line:
                sumarize[line] = None

            else:
                print(line)

            print(len(sumarize), sumarize)

if __name__ == '__main__':
    sys.exit(0 if main() else 1)