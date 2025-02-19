/**
 * @file slog.h
 * @brief Serial logger functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#pragma once
#include "usart.h"
#include <stdarg.h>

#define SLOG_BUFFER_SIZE 128
#define SLOG_ERROR(...)   slog_transmit(LOG_LEVEL_ERROR, __VA_ARGS__)
#define SLOG_WARN(...) slog_transmit(LOG_LEVEL_WARN, __VA_ARGS__)
#define SLOG_DEBUG(...)   slog_transmit(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define SLOG_INFO(...)    slog_transmit(LOG_LEVEL_INFO, __VA_ARGS__)

typedef enum {
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_INFO = 4,
} slog_level_t;

void slog_transmit(slog_level_t level, const char* format, ...);
