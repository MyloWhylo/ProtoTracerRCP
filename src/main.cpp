#include <stdio.h>

#include "LED.h"
#include "ProtoSPI.h"
#include "pico/stdlib.h"
#include "tusb.h"

#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define NUM_MATRICES 1
#define NUM_PIXELS MATRIX_HEIGHT* MATRIX_WIDTH* NUM_MATRICES

int main() {
    stdio_init_all();

#ifndef PICO_DEFAULT_LED_PIN
#warning ProtoTracer Reality Coprocessor requires the builtin LED for status.
#endif

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_init(15);  // CS IRQ Pulse
    gpio_set_dir(15, GPIO_OUT);

    gpio_init(14);  // DMA IRQ Pulse
    gpio_set_dir(14, GPIO_OUT);
    // while (!tud_cdc_connected()) {  // Wait for USB to connect
    //     sleep_ms(100);
    //     gpio_xor_mask(PICO_DEFAULT_LED_PIN);  // hehe toggle
    // }

    gpio_put(PICO_DEFAULT_LED_PIN, false);
    printf("ProtoTracer Reality Coprocessor initializing...\n");

    printf("Matrix Initialization: ");
    PixelMatrix myloMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, true, true);  // = new PixelMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, true);
    myloMatrix.init();
    myloMatrix.setBrightness(64);
    printf("OK!\n\n");

    printf("Initializing SPI...\n");
    ProtoSPI protoTransfer(NUM_PIXELS, &myloMatrix);
    protoTransfer.init();
    printf("SPI Initialized!\n\n");

    printf("Emotion Engine to Reality Coprocessor DMA Bridge established, beginning display.\n");

    while (true) {
    }
}
