/**
 * @file hal.c
 * @brief Connecting logic to HAL callbacks
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "indev.h"
#include "screen.h"
#include "memory.h"
#include "slog.h"
#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

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
    if (hspi == &memory_spi) {
        memory_tx_complete_handler();
    }
}

/**
 * @brief SPI receive complete handler
 *
 * @param hspi SPI handle
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi) {
    if (hspi == &memory_spi) {
        memory_rx_complete_handler();
    }
}

/**
 * @brief UART transmit complete handler
 *
 * @param huart UART handle
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart == &slog_uart) {
        slot_tx_complete_handler();
    }
}
