#pragma once
#ifndef LED_MYLO
#define LED_MYLO

#include <stdint.h>
#include <stdio.h>

#include "./ws2812.pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#define PIXEL_CLOCK 800000.0f
#define PIXEL_TIME 1.25f
#define BITS_PER_PIXEL 24
#define RESET_TIME 75

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 2
#endif

const uint8_t gamma8[] = {
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
	 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
	 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
	 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	 90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
	 115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
	 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
	 177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
	 215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};

class Pixel {
  public:
	uint32_t grb;

	Pixel() {
		this->grb = 0x00;
	}

	Pixel(uint8_t red, uint8_t green, uint8_t blue) {
		this->setPixel(red, green, blue);
	}

	void setPixel(uint8_t red, uint8_t green, uint8_t blue) {
		this->grb = (green << 24) | (red << 16) | (blue << 8);
	}

	void setPixel(uint32_t hex) {
		uint8_t r = hex & 0x00FF0000;
		uint8_t g = (hex & 0x0000FF00) << 16;
		uint8_t b = (hex & 0x000000FF) << 8;
		this->grb = g | r | b;
	}
};

// I need this array to be contiguous so that the DMA works properly
// ughhugughghguhgughg i hate programming
class PixelMatrix {
  private:
	uint pin;
	Pixel* frontPage;
	Pixel* backPage;
	bool snake;
	uint64_t timeBetweenFrames;
	absolute_time_t nextFrameTime = nil_time;

	float brightness = 0.20f;
	bool doGamma = true;

	int dmaChannel;
	PIO pio = pio0;
	int sm = 0;

	void initializeDMA() {
		dma_channel_config c = dma_channel_get_default_config(this->dmaChannel);
		channel_config_set_read_increment(&c, true);
		channel_config_set_write_increment(&c, false);
		channel_config_set_dreq(&c, pio_get_dreq(this->pio, this->sm, true));

		uint transferCnt = this->w * this->h;

		dma_channel_configure(dmaChannel, &c,
									 &pio->txf[this->sm],  // Destination pointer
									 this->frontPage,      // Source pointer
									 transferCnt,          // Number of transfers
									 false                 // Start immediately
		);
	}

  public:
	uint w;
	uint h;

	PixelMatrix(uint width, uint height, bool snake, bool gamma) {
		this->w = width;
		this->h = height;
		this->snake = snake;
		this->pin = WS2812_PIN;
		this->doGamma = gamma;
		this->frontPage = new Pixel[height * width];
		this->backPage = new Pixel[height * width];
	}

	PixelMatrix(uint width, uint height, bool snake, uint pin, bool gamma) {
		this->w = width;
		this->h = height;
		this->snake = snake;
		this->pin = pin;
		this->doGamma = gamma;
		this->frontPage = new Pixel[height * width];
		this->backPage = new Pixel[height * width];
	}

	void init() {
		uint offset = pio_add_program(this->pio, &ws2812_program);
		this->dmaChannel = dma_claim_unused_channel(true);
		this->sm = pio_claim_unused_sm(this->pio, true);
		myloWSInit(this->pio, this->sm, offset, PIXEL_CLOCK, pin);
		this->timeBetweenFrames = (this->w * this->h * BITS_PER_PIXEL * PIXEL_TIME) + RESET_TIME;
		this->initializeDMA();
	}

	void clear() {
		for (int ii = 0; ii < (this->w * this->h); ii++) {
			this->backPage[ii].setPixel(0, 0, 0);
		}
	}

	Pixel* getPixel(uint x, uint y) {
		if (this->snake) {
			return &(this->backPage[(y * w) + ((y % 2) ? (this->w - 1) - x : x)]);
		} else {
			return &(this->backPage[(y * w) + x]);
		}
	}

	void setPixel(uint index, uint8_t r, uint8_t g, uint8_t b) {
		if (this->doGamma) {
			this->backPage[index].setPixel(gamma8[r] * brightness, gamma8[g] * brightness, gamma8[b] * brightness);
		} else {
			this->backPage[index].setPixel(r * brightness, g * brightness, b * brightness);
		}
	}

	void setPixel(uint x, uint y, uint8_t r, uint8_t g, uint8_t b) {
		if (this->doGamma) {
			this->getPixel(x, y)->setPixel(gamma8[r] * brightness, gamma8[g] * brightness, gamma8[b] * brightness);
		} else {
			this->getPixel(x, y)->setPixel(r * brightness, g * brightness, b * brightness);
		}
	}

	void setPixel(uint x, uint y, uint32_t hex) {
		this->getPixel(x, y)->setPixel(hex);
	}

	void setBrightness(uint newBright) {
		this->brightness = newBright / 256.0f;
	}

	void display() {
		if (absolute_time_diff_us(get_absolute_time(), this->nextFrameTime) <= 0) {
			sleep_until(nextFrameTime);
		}

		Pixel* tmp = this->backPage;
		this->backPage = this->frontPage;
		this->frontPage = tmp;

		dma_channel_set_read_addr(this->dmaChannel, this->frontPage, true);
		this->nextFrameTime = make_timeout_time_us(this->timeBetweenFrames);
	}

	bool isBusy() {
		return dma_channel_is_busy(this->dmaChannel);
	}
};
#endif