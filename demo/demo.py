import cv2
from spi import RCP
import time
import numpy as np
from interval import PeriodicSleeper

thisRCP = RCP()
thisRCP.initializeRCP()
time.sleep(1)

print("Opening file")
vidcap = cv2.VideoCapture('nggyu.mp4')
success, frame = vidcap.read()
height, width, channels = frame.shape
print(f'H: {height}, W: {width}, D: {channels}')

print("Determining scale factor")
scaleFactor = 16/width if width < height else 16/height

dim = (int(width * scaleFactor), int(height * scaleFactor))
leftBound = int((dim[0] - 16) / 2)
print(f'New dimensions: {dim[0]}x{dim[1]}, starting at {leftBound}.')

print("Starting video transfer")


def frameFunction():
    global frame
    frame = cv2.resize(frame, dim, interpolation=cv2.INTER_AREA)
    # frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    frame = frame[:, leftBound:leftBound+16, :]
    frame = cv2.flip(frame, 1)
    # print(frame.shape)
    # frame = np.repeat(frame, 3)
    # print(frame.shape)
    # print(frame.tobytes())

    thisRCP.sendData(frame)
    # cv2.imwrite("frame%d.jpg" % count, frame)     # save frame as JPEG file
    success, frame = vidcap.read()
    return success


sleeper = PeriodicSleeper(frameFunction, 25)
