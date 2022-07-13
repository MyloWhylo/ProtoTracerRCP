import cv2
import sys
from spi import RCP
from interval import PeriodicSleeper

success, frame = None


def frameFunction():
    global frame
    frame = cv2.resize(frame, dim, interpolation=cv2.INTER_AREA)
    frame = frame[:, leftBound:leftBound+16, :]
    frame = cv2.flip(frame, 1)

    thisRCP.sendData(frame)
    success, frame = vidcap.read()
    return success


if __name__ == "__main__":
    inputVideoFile = "demo.mp4"
    argvIter = iter(sys.argv)

    print("### ProtoTracer RCP Demo ###")
    for arg in argvIter:
        match arg:
            case '-i':
                inputVideoFile = next(argvIter)

    print("Initializing RCP... ", end="")
    sys.stdout.flush()

    thisRCP = RCP()
    thisRCP.initializeRCP()
    print("OK!\n")

    print("Opening file...")
    vidcap = cv2.VideoCapture(inputVideoFile)
    success, frame = vidcap.read()
    height, width, channels = frame.shape
    print(f'H: {height}, W: {width}, D: {channels}')

    print("Determining scale factor...")
    scaleFactor = 16/width if width < height else 16/height

    dim = (int(width * scaleFactor), int(height * scaleFactor))
    leftBound = int((dim[0] - 16) / 2)

    print(f'New dimensions: {dim[0]}x{dim[1]}, starting at {leftBound}.')
    print("Starting video transfer.")

    sleeper = PeriodicSleeper(frameFunction, 25)
