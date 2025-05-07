/**
 * @file indev.h
 * @brief Input device functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gpio.h"

/**
 * @brief Input device interrupt requests handler
 *
 * @param GPIO_Pin Pin that triggered request
 */
void indev_irq_handler(uint16_t GPIO_Pin);