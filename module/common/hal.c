/**
 * @file hal.c
 * @brief Connecting logic to HAL callbacks
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "indev.h"
#include "screen.h"
#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"

/**
 * @brief Interrupt requests handler
 *
 * @param GPIO_Pin Pin that triggered request
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    indev_irq_handler(GPIO_Pin);
}

/**
 * @brief SPI transmit complete handler
 * 
 * @param hspi SPI handle
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
    if (hspi == &screen_spi) {
        screen_tx_complete_handler();
    }
}