# USB Dongle tester and flasher

pouzit je program https://github.com/richardeoin/ftx-prog

firmware pro test https://bitbucket.org/BigClown/bcf-tester-usb-dongle

## Install

sudo apt install libftdi-dev python3-pip libopenjp2-7 python3-pil

sudo pip3 install -r requirements.txt


## Seznam coru

https://docs.google.com/spreadsheets/d/1fuPB4bwQ8_RaiHlOcM8E4XiPLnKGJNKt6omWwRMIUsE/edit


## Testovani

Spustit program a řídit se instrukcemi

### Pro testovani na pc

        cd bch-tester-core2
        ./run.py --firmware bigclownlabs/bcf-radio-climate-monitor:latest

### Pro testovani na rpi

        cd bch-tester-core2
        sudo -s HOME=/home/pi ./run.py --firmware bigclownlabs/bcf-radio-motion-detector:latest
