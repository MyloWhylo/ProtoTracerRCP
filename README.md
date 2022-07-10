# ProtoTracer Reality Coprocessor
A Pi Pico-based WS2812b display driver that recieves data via SPI

## Background
This is designed to take frame data sent via SPI to a Pi Pico and display it on a WS2812b LED matrix. I designed it mainly because I wanted to see if I could.

### Naming
> Why "Reality Coprocessor"?

This has absolutely nothing to do with the N64's RCP, I just liked the name. I decided to call the Pi 4 that drives graphics the "emotion engine" and the microcontroller that drives the displays the "reality coprocessor", because it felt clever (and trademark-infringey lol)

## Protocol
I decided to have a little bit of smarts baked in instead of having a totally undocumented protocol. There's two stages of operation, **Initialization** and **Transfer**.

### Initialization
In this mode, the PTRCP behaves kind of like an I2C device, in that you send it an address and it'll respond with some data. In particular, there are 5 registers and 1 command:
| **Address** | **Name**        | **Description**                                                                               |
|-------------|-----------------|-----------------------------------------------------------------------------------------------|
| 0x00        | Device ID       | Returns the ID of the device (by default, 0x05)                                               |
| 0x01        | Panel Width Lo  | Low byte of panel width.                                                                      |
| 0x02        | Panel Width Hi  | High byte of panel width. Reserved for a Hub75 version of the RCP.                            |
| 0x03        | Panel Height Lo | Low byte of panel height.                                                                     |
| 0x04        | Panel Height Hi | High byte of panel height. Reserved for a Hub75 version of the RCP.                           |
| 0x05        | Maximum FPS     | Returns the maximum FPS supported by the device. This is usually an SPI bandwidth bottleneck. |
| 0x2A        | Enter gDMA mode | Command exits initialization mode and enters graphics transfer mode.                          |

### Transfer
In this mode, the SPI engine transfers the number of data bytes required for a single frame, and then draws it to the screen. The data is sent from first pixel to last, in RGB order. This is to make it more universal, as if the data was sent in GRB order (how WS2812b pixels are) then it would be marginally more difficult to adapt to more standard panels later on. 

## Design
By design, this uses as many background features of the RP2040 as possible. I use DMA to transfer from the SPI module to RAM, and then trigger an interrupt to write the data into a framebuffer. The WS2812b interface uses one of the PIOs and another DMA channel to transfer the data out in the background, without any CPU load.
