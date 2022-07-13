#!/usr/bin/env python3
import spidev
import time
import numpy as np
import sys


class RCP:
    def __init__(self, spi_device=0, ce_pin=0, speed=1000000):
        self.spi = spidev.SpiDev()
        self.spi.open(spi_device, ce_pin)
        self.spi.max_speed_hz = speed
        self.spi.mode = 0b11

    def initializeRCP(self):
        count = 0
        print("Initializing communications...")

        if self.__validateRCP() == False:
            print("Validation failed!")
            exit()
        
        print("Found Reality Coprocessor!")
        print(f'Width: {self.width}, Height: {self.height}, Framerate: {self.framerate}')
        print("Switching to gDMA mode, RCP is ready for frame data.")

    def __readRegister(self, address):
        read_bytes = self.spi.xfer2([address, 0x00], 115200)
        return (read_bytes[1])

    def __getConfigHeader(self):
        read_bytes = self.spi.xfer(
            [0x00, 0x00, 0x00, 0x00, 0x00, 0x00], 115200)
        return read_bytes

    def __getDeviceID(self):
        regVal = self.__readRegister(0x00)
        return hex(regVal)

    def __validateRCP(self):
        header = self.__getConfigHeader()
        if header[0] != 0x05:
            print("Invalid header!")
            return False

        self.width = header[1] | (header[2] << 8)
        self.height = header[3] | (header[4] << 8)
        self.framerate = header[5]

        if self.width > 0 and self.height > 0 and self.framerate > 0:
            return True
        else:
            return False

    def __enableGDMA(self):
        self.spi.xfer2([42, 0, 0, 0, 0, 0], 250000)

    def sendData(self, frame):
        self.spi.xfer2(frame.tobytes(), 4000000, 0, 8)


if __name__ == "__main__":
    thisRCP = RCP()
    thisRCP.initializeRCP()
