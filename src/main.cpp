#include <stdio.h>

#include "LED.h"
#include "ProtoSPI.h"
#include "pico/stdlib.h"

#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define NUM_MATRICES 1
#define NUM_PIXELS MATRIX_HEIGHT* MATRIX_WIDTH* NUM_MATRICES

int main() {
// All of this is for aesthetics
#ifndef PICO_DEFAULT_LED_PIN
#warning ProtoTracer Reality Coprocessor requires the builtin LED for status.
#else
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif

	printf("ProtoTracer Reality Coprocessor initializing...");
	stdio_init_all();

	// for (int ii = 0; ii < 30; ii++) {
	// 	printf(".");
	// 	gpio_put(PICO_DEFAULT_LED_PIN, 1);
	// 	sleep_ms(125);
	// 	gpio_put(PICO_DEFAULT_LED_PIN, 0);
	// 	sleep_ms(125);
	// }

	// sleep_ms(750);
	printf("\n");

	printf("Matrix Initialization: ");
	PixelMatrix myloMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, true, true);  // = new PixelMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, true);
	myloMatrix.init();
	myloMatrix.setBrightness(64);
	printf("OK!\n\n");

	printf("Initializing SPI...\n");
	ProtoSPI protoTransfer(NUM_PIXELS, &myloMatrix);
	protoTransfer.init(1000000);
	printf("SPI Initialized!\n\n");

	printf("Emotion Engine to Reality Coprocessor DMA Bridge established, beginning display.\n");

	while (true) {
	}
}
