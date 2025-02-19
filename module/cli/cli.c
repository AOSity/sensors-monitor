/**
 * @file cli.c
 * @brief Console line interface
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "slog.h"
#include "main.h"
#include "cmsis_os.h"

static uint8_t rx_buffer[1] = { 0 };


void cli_task(void* argument)
{
    HAL_UART_Receive_IT(&slog_uart, rx_buffer, 1);
    for (;;) {
        SLOG_INFO("cli working...");
        osDelay(5000);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart == &slog_uart) {
        HAL_UART_Receive_IT(&slog_uart, rx_buffer, 1);
        switch (rx_buffer[0]) {
        case '1':
            HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
            break;
        case '2':
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            break;
        case '3':
            HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
            break;
        }
    }
}
