/**
 * @file st7796s.c
 * @brief st7796s screen IC driver functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "screen.h"
#include "spi.h"
#include "main.h"
#include "gpio.h"
#include "cmsis_os2.h"

#include <stdbool.h>

#define DC_Command() HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)
#define DC_Data()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
#define CS_Enable()  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define CS_Disable() HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)

static void st7796s_init(void);
static void st7796s_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void st7796s_send_pixels(const uint16_t* data, size_t size);

screen_driver_t screen;
void screen_init_driver(void) {
    screen.init = st7796s_init;
    screen.set_window = st7796s_set_window;
    screen.send_pixels = st7796s_send_pixels;
}

static void send_cmd(uint8_t cmd) {
    DC_Command();
    CS_Enable();
    HAL_SPI_Transmit(&screen_spi, &cmd, 1, HAL_MAX_DELAY);
    CS_Disable();
}

static void send_data(uint8_t data) {
    DC_Data();
    CS_Enable();
    HAL_SPI_Transmit(&screen_spi, &data, 1, HAL_MAX_DELAY);
    CS_Disable();
}

static void st7796s_init(void) {
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    osDelay(40);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    osDelay(120);

    send_cmd(0x11);
    osDelay(120);

    send_cmd(0x3A); 
    send_data(0x55); // Interface Pixel Format = 16 bit

    send_cmd(0x36); 
    send_data(0x00); // Memory Access Control

    send_cmd(0x29); // Display ON
    osDelay(10);
}

static void st7796s_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    send_cmd(0x2A);
    send_data(x0 >> 8); send_data(x0 & 0xFF);
    send_data(x1 >> 8); send_data(x1 & 0xFF);

    send_cmd(0x2B);
    send_data(y0 >> 8); send_data(y0 & 0xFF);
    send_data(y1 >> 8); send_data(y1 & 0xFF);

    send_cmd(0x2C);
}

static volatile bool dma_done = true;
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
    if (hspi == &screen_spi) {
        dma_done = true;
    }
}

static void wait_dma_ready(void) {
    while (!dma_done) {
        osDelay(1);
    }
}

static void st7796s_send_pixels(const uint16_t* data, size_t size) {
    wait_dma_ready();
    dma_done = false;

    DC_Data();
    CS_Enable();
    HAL_SPI_Transmit_DMA(&screen_spi, (uint8_t*)data, size * 2);
    wait_dma_ready();
    CS_Disable();
}
