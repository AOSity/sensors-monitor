/**
 * @file slog.c
 * @brief Serial logger functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "slog.h"
#include "main.h"

#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef* uart_handle = &slog_uart;

/**
 * @brief Transmit log message
 *
 * @param level log level
 * @param format format string
 * @param ...  argv
 */
void slog_transmit(slog_level_t level, const char* format, ...) {
    if (!uart_handle) {
        return;
    }
    char buffer[SLOG_BUFFER_SIZE];
    int offset = 0;

    switch (level) {
    case LOG_LEVEL_ERROR:
        offset = snprintf(buffer, SLOG_BUFFER_SIZE, "[ERROR] ");
        break;
    case LOG_LEVEL_WARN:
        offset = snprintf(buffer, SLOG_BUFFER_SIZE, "[WARNING] ");
        break;
    case LOG_LEVEL_DEBUG:
        offset = snprintf(buffer, SLOG_BUFFER_SIZE, "[DEBUG] ");
        break;
    case LOG_LEVEL_INFO:
        offset = snprintf(buffer, SLOG_BUFFER_SIZE, "[INFO] ");
        break;
    default:
        offset = 0;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(buffer + offset, SLOG_BUFFER_SIZE - offset, format, args);
    va_end(args);

    strncat(buffer, "\r\n", SLOG_BUFFER_SIZE - strlen(buffer) - 1);

    HAL_UART_Transmit(uart_handle, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}
