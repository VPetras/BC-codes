import smbus
import time

BC_MODULE_RELAY_POLARITY_F    = ((1 << 6) | (1 << 7))
BC_MODULE_RELAY_POLARITY_T    = ((1 << 4) | (1 << 5))
BC_MODULE_RELAY_POLARITY_NONE = ((1 << 6) | (1 << 4))

BC_TCA9534A_REGISTER_INPUT_PORT = 0x00
BC_TCA9534A_REGISTER_OUTPUT_PORT = 0x01
BC_TCA9534A_REGISTER_POLARITY_INVERSION = 0x02
BC_TCA9534A_REGISTER_CONFIGURATION = 0x03

class Relay_module():
    def __init__(self):
        self.bus = smbus.SMBus(1)
        
        self.address = 0x3b
        
        self.bus.write_byte_data(self.address, BC_TCA9534A_REGISTER_OUTPUT_PORT, BC_MODULE_RELAY_POLARITY_NONE)

        self.bus.write_byte_data(self.address, BC_TCA9534A_REGISTER_CONFIGURATION, 0x00)
    
    def state(self, state):

        if state:
            self.bus.write_byte_data(self.address, BC_TCA9534A_REGISTER_OUTPUT_PORT, BC_MODULE_RELAY_POLARITY_T)
        else:
            self.bus.write_byte_data(self.address, BC_TCA9534A_REGISTER_OUTPUT_PORT, BC_MODULE_RELAY_POLARITY_F)

        time.sleep(0.1)

        self.bus.write_byte_data(self.address, BC_TCA9534A_REGISTER_OUTPUT_PORT, BC_MODULE_RELAY_POLARITY_NONE)


def main():
    relay = Relay_module()

    for i in range(10):
        relay.state(i % 2 == 0)
        time.sleep(1)

if __name__ == '__main__':
    import sys
    sys.exit(0 if main() else 1)