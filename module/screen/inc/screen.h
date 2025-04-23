/**
 * @file screen.h
 * @brief Screen driver interface
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    void (*init)(void);
    void (*set_window)(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void (*send_pixels)(const uint16_t* data, size_t size);
} screen_driver_t;

extern screen_driver_t screen;

/**
 * @brief Fills screen driver structure
 * @note Shall be implemented in you screen IC driver source file
 */
void screen_init_driver(void);
