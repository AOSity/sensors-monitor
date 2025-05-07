/**
 * @file ft6336u.c
 * @brief ft6336u touch IC driver functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "indev.h"
#include "slog.h"
#include "i2c.h"
#include "main.h"

#include <stdbool.h>

static volatile bool touch_pending = false;

/**
 * @brief Input device interrupt requests handler
 *
 * @param GPIO_Pin Pin that triggered request
 */
void indev_irq_handler(uint16_t GPIO_Pin) {
    if (GPIO_Pin == TCH_IRQ_Pin) {
        touch_pending = true;
    }
}
