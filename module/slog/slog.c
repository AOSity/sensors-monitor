/**
 * @file slog.c
 * @brief Serial logger functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "slog.h"
#include "main.h"
#include "cmsis_os2.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define LOG_BUFFER_SIZE 1024
#define LOG_MESSAGE_SIZE 128

static uint8_t log_buffer[LOG_BUFFER_SIZE];
static volatile uint16_t log_head = 0;
static volatile uint16_t log_tail = 0;
static volatile bool dma_ready = true;

/**
 * @brief Handle transmit complete to update buffer tail and dma ready flag 
 * 
 * @param huart uart handle
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart == &slog_uart) {
        uint16_t len = (log_head >= log_tail) ? log_head - log_tail : LOG_BUFFER_SIZE - log_tail;
        log_tail = (log_tail + len) % LOG_BUFFER_SIZE;
        dma_ready = true;
    }
}

/**
 * @brief Prepare message and write to logs buffer
 *
 * @param level log level
 * @param format format string
 * @param ...  argv
 */
void slog_write(slog_level_t level, const char* format, ...) {
    char msg[LOG_MESSAGE_SIZE];
    int offset = 0;

    switch (level) {
    case LOG_LEVEL_ERROR:
        offset = snprintf(msg, LOG_MESSAGE_SIZE, "[ERROR] ");
        break;
    case LOG_LEVEL_WARN:
        offset = snprintf(msg, LOG_MESSAGE_SIZE, "[WARN] ");
        break;
    case LOG_LEVEL_DEBUG:
        offset = snprintf(msg, LOG_MESSAGE_SIZE, "[DEBUG] ");
        break;
    case LOG_LEVEL_INFO:
        offset = snprintf(msg, LOG_MESSAGE_SIZE, "[INFO] ");
        break;
    default:
        offset = 0;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(msg + offset, LOG_MESSAGE_SIZE - offset, format, args);
    va_end(args);
    strncat(msg, "\r\n", LOG_MESSAGE_SIZE - strlen(msg) - 1);

    for (size_t i = 0; i < strlen(msg); i++) {
        uint16_t next = (log_head + 1) % LOG_BUFFER_SIZE;
        if (next == log_tail) {
            /* buffer full */
            break;
        }
        log_buffer[log_head] = msg[i];
        log_head = next;
    }
}

/**
 * @brief Periodically transmits logs buffer via slog_uart
 */
void slog_print_task(void* unused) {
    SLOG_DEBUG("slog_print_task started");
    for (;;) {
        if (dma_ready && log_head != log_tail) {
            uint16_t len = (log_head >= log_tail) ? log_head - log_tail : LOG_BUFFER_SIZE - log_tail;
            if (len > 0) {
                dma_ready = false;
                HAL_UART_Transmit_DMA(&slog_uart, &log_buffer[log_tail], len);
            }
        }
        osDelay(10);
    }
}
