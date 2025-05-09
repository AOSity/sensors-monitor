/**
 * @file indev.h
 * @brief Input device functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gpio.h"
#include <stdbool.h>

typedef enum {
    INDEV_TYPE_TOUCH,
} indev_type_t;

typedef struct {
    indev_type_t type;
    union {
        struct {
            void (*read)(void);
            uint16_t x;
            uint16_t y;
            bool touched;
        } touch;
    };
} indev_t;

extern indev_t indev;

/**
 * @brief Fills indev structure
 * @note Shall be implemented in you input device driver source file
 */
void indev_init(void);

/**
 * @brief Input device interrupt requests handler
 *
 * @param GPIO_Pin Pin that triggered request
 */
void indev_irq_handler(uint16_t GPIO_Pin);