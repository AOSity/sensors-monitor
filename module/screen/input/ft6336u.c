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
#include "cmsis_os2.h"

#include <stdbool.h>

#define FT6336U_ADDR (0x38 << 1)
#define FT6336U_REG_TD_STATUS 0x02
#define FT6336U_REG_P1_XH     0x03

indev_t indev;
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

static void ft6336u_init(void);
static void ft6336u_read_touch(void);

void indev_init(void) {
    ft6336u_init();
    indev.type = INDEV_TYPE_TOUCH;
    indev.touch.read = ft6336u_read_touch;
}

static void ft6336u_init(void) {
    HAL_GPIO_WritePin(TCH_RST_GPIO_Port, TCH_RST_Pin, GPIO_PIN_RESET);
    osDelay(40);
    HAL_GPIO_WritePin(TCH_RST_GPIO_Port, TCH_RST_Pin, GPIO_PIN_SET);
    osDelay(120);
}

static void ft6336u_read_touch(void) {
    if (!touch_pending) {
        return;
    }
    touch_pending = false;

    uint8_t points;
    HAL_I2C_Mem_Read(&touch_i2c, FT6336U_ADDR, FT6336U_REG_TD_STATUS, 1, &points, 1, HAL_MAX_DELAY);
    points &= 0x0F;
    if (points > 0) {
        uint8_t data[4];
        HAL_I2C_Mem_Read(&touch_i2c, FT6336U_ADDR, FT6336U_REG_P1_XH, 1, data, 4, HAL_MAX_DELAY);
        indev.touch.touched = true;
        indev.touch.x = ((data[0] & 0x0F) << 8) | data[1];
        indev.touch.y = ((data[2] & 0x0F) << 8) | data[3];
    } else {
        indev.touch.touched = false;
    }
}
