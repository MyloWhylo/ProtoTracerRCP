#include <stdint.h>
#include <stdio.h>

#include "LED.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

volatile bool handlerDidFinish = true;
uint rxChan;
uint bufferLen;
uint8_t* rxBuf;
PixelMatrix* dispMatrix;

void errorblink();
static void spiTransferHandler();

static void spiTransferHandler() {
   if (!handlerDidFinish) {
      dma_channel_set_irq0_enabled(rxChan, false);
      irq_remove_handler(DMA_IRQ_0, spiTransferHandler);
      errorblink();
   }

   handlerDidFinish = false;
   uint bufferIndex = 0;
   for (uint ii = 0; ii < bufferLen; ii++) {
      uint8_t r = rxBuf[bufferIndex++];
      uint8_t g = rxBuf[bufferIndex++];
      uint8_t b = rxBuf[bufferIndex++];
      dispMatrix->setPixel(ii, r, g, b);
   }

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

      dma_channel_configure(this->recieveChannel, &rxCfg,
                            this->recieveBuf,       // Destination pointer
                            &spi_get_hw(spi0)->dr,  // Source pointer
                            bufLen,                 // Number of transfers
                            false                   // Start immediately
      );

      dma_channel_configure(this->transmitChannel, &txCfg,
                            &spi_get_hw(spi0)->dr,  // Destination pointer
                            &this->dummyChar,       // Source pointer
                            bufLen,                 // Number of transfers
                            false                   // Start immediately
      );

      dma_channel_set_irq0_enabled(this->recieveChannel, true);
      irq_set_exclusive_handler(DMA_IRQ_0, spiTransferHandler);
      irq_set_priority(DMA_IRQ_0, 0b01000000);  // Higher priority, but not highest
   }

  public:
   ProtoSPI(uint numPixels, PixelMatrix* outMatrix) {
      this->bufLen = numPixels * 3;

      bufferLen = numPixels;
      dispMatrix = outMatrix;
      this->recieveBuf = new uint8_t[numPixels * 3];
      rxBuf = this->recieveBuf;
   }

   void init(uint frequency) {
      printf("\tSPI 0: ");
      spi_init(spi0, frequency);
      spi_set_slave(spi0, true);
      printf("OK!\n");

      uint8_t panelWL = dispMatrix->w & 0xFF;
      uint8_t panelWH = dispMatrix->w & 0xFF00;
      uint8_t panelHL = dispMatrix->h & 0xFF;
      uint8_t panelHH = dispMatrix->h & 0xFF00;
      uint8_t deviceID = 0x05;

      // Handshake Transfer
      printf("\tBeginning handshake...\n");
      while (true) {
         uint8_t command;
         spi_read_blocking(spi0, 0x00, &command, 1);

         if (command == 0x42) break;
         switch (command) {
            case 0x00:  // Device ID
               spi_write_blocking(spi0, &deviceID, 1);
               break;

            case 0x01:  // Panel Width Low
               spi_write_blocking(spi0, &panelWL, 1);
               break;

            case 0x02:  // Panel Width Hi
               spi_write_blocking(spi0, &panelWL, 1);
               break;

            case 0x03:  // Panel Height Low
               spi_write_blocking(spi0, &panelHL, 1);
               break;

            case 0x04:  // Panel Height Hi
               spi_write_blocking(spi0, &panelHH, 1);
               break;

            default:
               break;
         }
      }
      printf("\tSuccessfully communicated with the ProtoTracer Emotion Engine!\n");
      printf("\tInitializing Graphis DMA... ");
      this->initializeDMA();
      printf("OK!\n");
   }
};