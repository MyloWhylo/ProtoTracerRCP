#include <stdint.h>
#include <stdio.h>

#include "LED.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

volatile bool handlerDidFinish = true;
uint rxChan;
uint txChan;
uint bufferLen;
uint8_t* rxBuf;
PixelMatrix* dispMatrix;

void errorblink();
static void spiTransferHandler();

static void spiTransferHandler() {
	if (!handlerDidFinish) {
		printf("Handler didn't finish!");
		dma_channel_set_irq0_enabled(rxChan, false);
		irq_remove_handler(DMA_IRQ_0, spiTransferHandler);
		errorblink();
	}

	// Update framebuffer
	handlerDidFinish = false;
	uint bufferIndex = 0;
	for (uint ii = 0; ii < bufferLen; ii++) {
		uint8_t r = rxBuf[bufferIndex++];
		uint8_t g = rxBuf[bufferIndex++];
		uint8_t b = rxBuf[bufferIndex++];
		dispMatrix->setPixel(ii, r, g, b);
	}

	// Restart DMA
	dma_channel_set_write_addr(rxChan, rxBuf, false);
	dma_start_channel_mask((1u << txChan) | (1u << rxChan));

	// Display Framebuffer
	dispMatrix->display();
	handlerDidFinish = true;
}

void errorblink() {
	while (true) {
		gpio_put(PICO_DEFAULT_LED_PIN, 1);
		sleep_ms(100);
		gpio_put(PICO_DEFAULT_LED_PIN, 0);
		sleep_ms(100);
	}
}

class ProtoSPI {
  private:
	uint bufLen;
	uint8_t dummyChar = 0x1B;
	uint8_t* recieveBuf;
	int recieveChannel;
	int transmitChannel;

	void initializeDMA() {
		this->recieveChannel = dma_claim_unused_channel(true);
		rxChan = this->recieveChannel;
		this->transmitChannel = dma_claim_unused_channel(true);
		txChan = this->transmitChannel;

		dma_channel_config rxCfg = dma_channel_get_default_config(this->recieveChannel);
		channel_config_set_transfer_data_size(&rxCfg, DMA_SIZE_8);
		channel_config_set_read_increment(&rxCfg, false);
		channel_config_set_write_increment(&rxCfg, true);
		channel_config_set_dreq(&rxCfg, spi_get_dreq(spi0, false));

		dma_channel_config txCfg = dma_channel_get_default_config(this->transmitChannel);
		channel_config_set_transfer_data_size(&txCfg, DMA_SIZE_8);
		channel_config_set_read_increment(&txCfg, false);
		channel_config_set_write_increment(&txCfg, false);
		channel_config_set_dreq(&txCfg, spi_get_dreq(spi0, true));

		dma_channel_set_irq0_enabled(this->recieveChannel, true);
		irq_set_exclusive_handler(DMA_IRQ_0, spiTransferHandler);
		// irq_set_priority(DMA_IRQ_0, PICO_HIGHEST_IRQ_PRIORITY);  // Higher priority, but not highest
		irq_set_enabled(DMA_IRQ_0, true);

		dma_channel_configure(this->recieveChannel, &rxCfg,
									 this->recieveBuf,       // Destination pointer
									 &spi_get_hw(spi0)->dr,  // Source pointer
									 this->bufLen,           // Number of transfers
									 false                   // Start immediately
		);

		dma_channel_configure(this->transmitChannel, &txCfg,
									 &spi_get_hw(spi0)->dr,  // Destination pointer
									 &this->dummyChar,       // Source pointer
									 this->bufLen,           // Number of transfers
									 false                   // Start immediately
		);

		printf("\n\tStarting DMAs...\n");
		dma_start_channel_mask((1u << this->transmitChannel) | (1u << this->recieveChannel));
	}

  public:
	ProtoSPI(uint numPixels, PixelMatrix* outMatrix) {
		this->bufLen = numPixels * 3;
		printf("Expecting %d bytes.\n", this->bufLen);

		bufferLen = numPixels;
		dispMatrix = outMatrix;
		this->recieveBuf = new uint8_t[this->bufLen];
		rxBuf = this->recieveBuf;
	}

	void init(uint frequency) {
		printf("\tSPI 0: ");
		spi_init(spi0, 115200);
		spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
		spi_set_slave(spi0, true);

		gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
		gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
		gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
		gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
		printf("OK!\n");

		uint8_t panelWL = (dispMatrix->w & 0xFF);
		uint8_t panelWH = (dispMatrix->w >> 8) & 0xFF;
		uint8_t panelHL = (dispMatrix->h & 0xFF);
		uint8_t panelHH = (dispMatrix->h >> 8) & 0xFF;
		uint8_t fps = 120;
		uint8_t deviceID = 0x05;

		printf("Buffer data: %d %d %d %d %d %d\n", deviceID, panelWL, panelWH, panelHL, panelHH, fps);
		// Handshake Transfer
		printf("\tBeginning handshake...\n");
		uint8_t copi[6];
		uint8_t cipo[6] = {deviceID, panelWL, panelWH, panelHL, panelHH, fps};

		spi_write_read_blocking(spi0, cipo, copi, 6);

		printf("\tSuccessfully communicated with the ProtoTracer Emotion Engine!\n");
		printf("\tInitializing Graphcis DMA... ");

		spi_deinit(spi0);

		spi_init(spi0, 16000000);
		spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
		spi_set_slave(spi0, true);

		gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
		gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
		gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
		gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
		// this->initializeDMA();
		printf("OK!\n");

		while (true) {
			spi_read_blocking(spi0, 0x1b, this->recieveBuf, this->bufLen);
			uint bufferIndex = 0;
			for (uint_fast8_t yy = 0; yy < 16; yy++) {
				for (uint_fast8_t xx = 0; xx < 16; xx++) {
					uint8_t b = rxBuf[bufferIndex++];
					uint8_t g = rxBuf[bufferIndex++];
					uint8_t r = rxBuf[bufferIndex++];
					dispMatrix->setPixel(xx, yy, r, g, b);
				}
			}
			dispMatrix->display();
		}
	}
};