#!/usr/bin/env python3
import spidev
import RPi.GPIO as GPIO
import time
import numpy as np
import sys


class RCP:
    def __init__(self, spi_device=0, ce_pin=0, speed=16000000):
        self.spi = spidev.SpiDev()
        self.spi.open(spi_device, ce_pin)
        self.spi.max_speed_hz = speed
        self.spi.mode = 0b11

    def initializeRCP(self):
        print("Initializing communications...")
        # Resets the RCP, putting it in a known state
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(25, GPIO.OUT)

        GPIO.output(25, GPIO.LOW)
        time.sleep(0.05)
        GPIO.output(25, GPIO.HIGH)
        time.sleep(0.05)
        GPIO.cleanup()

        # Check if the RCP is present and valid
        if self.__validateRCP() == False:
            print("Validation failed!")
            exit()

        print("Found Reality Coprocessor!")
        print(f'Width: {self.width}, Height: {self.height}, Framerate: {self.framerate}')
        print("Switching to gDMA mode, RCP is ready for frame data.")

    def __readRegister(self, address):   # Unused
        read_bytes = self.spi.xfer2([address, 0x00], 115200)
        return (read_bytes[1])

    def __getConfigHeader(self):
        read_bytes = self.spi.xfer(
            [0x00, 0x00, 0x00, 0x00, 0x00, 0x00], 115200)
        return read_bytes

    def __getDeviceID(self):  # Unused
        regVal = self.__readRegister(0x00)
        return hex(regVal)

    def __validateRCP(self):    # make sure RCP is actually real
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
            print("Valid header, but data returned makes no sense!")
            print(f'H: {self.height}, W: {self.width}, F: {self.framerate}')
            return False

    def __enableGDMA(self):  # Unused
        self.spi.xfer2([42, 0, 0, 0, 0, 0], 250000)

    def sendData(self, frame):
        # delay for 500uSec to allow DMA to finish before CS goes high
        self.spi.xfer2(frame.tobytes(), 4000000, 500, 8)


if __name__ == "__main__":
    thisRCP = RCP()
    thisRCP.initializeRCP()
