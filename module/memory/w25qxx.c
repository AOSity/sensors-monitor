/**
 * @file w25qxx.c
 * @brief W25QXX serial flash memory driver
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "memory.h"
#include "spi.h"
#include "cmsis_os2.h"

#define W25QXX_CMD_READ_ID      0x9F
#define W25QXX_CMD_READ_DATA    0x03
#define W25QXX_CMD_PAGE_PROGRAM 0x02
#define W25QXX_CMD_SECTOR_ERASE 0x20
#define W25QXX_CMD_CHIP_ERASE   0xC7
#define W25QXX_CMD_WRITE_ENABLE 0x06

#define W25QXX_PAGE_SIZE        256
#define W25QXX_SECTOR_SIZE      4096

memory_driver_t memory;
static volatile bool dma_busy = false;

static void wait_dma_ready(void) {
    while (dma_busy) {
        osDelay(1);
    }
}

static void w25qxx_write_enable(void) {
    uint8_t cmd = W25QXX_CMD_WRITE_ENABLE;
    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&memory_spi, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);
}

static void w25qxx_wait_busy(void) {
    uint8_t cmd = 0x05, status;
    do {
        HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&memory_spi, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(&memory_spi, &status, 1, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);
        osDelay(1);
    } while (status & 0x01);
}

static void w25qxx_read(uint8_t* buf, uint32_t addr, uint32_t len) {
    wait_dma_ready();

    uint8_t cmd[4] = {
        W25QXX_CMD_READ_DATA,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF
    };

    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&memory_spi, cmd, 4, HAL_MAX_DELAY);
    dma_busy = true;
    HAL_SPI_Receive_DMA(&memory_spi, buf, len);
    wait_dma_ready();
    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);
}

static void w25qxx_write(const uint8_t* buf, uint32_t addr, uint32_t len) {
    wait_dma_ready();

    while (len > 0) {
        uint32_t page_offset = addr % W25QXX_PAGE_SIZE;
        uint32_t bytes_to_write = W25QXX_PAGE_SIZE - page_offset;
        if (bytes_to_write > len) bytes_to_write = len;

        w25qxx_write_enable();

        uint8_t cmd[4] = {
            W25QXX_CMD_PAGE_PROGRAM,
            (addr >> 16) & 0xFF,
            (addr >> 8) & 0xFF,
            addr & 0xFF
        };

        HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&memory_spi, cmd, 4, HAL_MAX_DELAY);
        dma_busy = true;
        HAL_SPI_Transmit_DMA(&memory_spi, (uint8_t*)buf, bytes_to_write);
        wait_dma_ready();
        HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);
        w25qxx_wait_busy();

        addr += bytes_to_write;
        buf += bytes_to_write;
        len -= bytes_to_write;
    }
}

static void w25qxx_erase_sector(uint32_t addr) {
    w25qxx_write_enable();

    uint8_t cmd[4] = {
        W25QXX_CMD_SECTOR_ERASE,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF
    };

    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&memory_spi, cmd, 4, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);

    w25qxx_wait_busy();
}

static void w25qxx_erase_chip(void) {
    w25qxx_write_enable();
    uint8_t cmd = W25QXX_CMD_CHIP_ERASE;

    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&memory_spi, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);

    w25qxx_wait_busy();
}

static uint32_t w25qxx_get_id(void) {
    uint8_t cmd = W25QXX_CMD_READ_ID;
    uint8_t id[3];

    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&memory_spi, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&memory_spi, id, 3, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);

    return (id[0] << 16) | (id[1] << 8) | id[2];
}

static bool w25qxx_init(void) {
    return (w25qxx_get_id() != 0);
}

void memory_init_driver(void) {
    memory.init = w25qxx_init;
    memory.read = w25qxx_read;
    memory.write = w25qxx_write;
    memory.erase_sector = w25qxx_erase_sector;
    memory.erase_chip = w25qxx_erase_chip;
    memory.get_id = w25qxx_get_id;
}

void memory_tx_complete_handler(void) {
    dma_busy = false;
}

void memory_rx_complete_handler(void) {
    dma_busy = false;
}
