/**
 * @file sensors.c
 * @brief Contains task for sensors detection and processing
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "main.h"
#include "i2c.h"
#include "slog.h"
#include "cmsis_os2.h"

void sensors_task(void* argument) {
    osDelay(150);
    SLOG_DEBUG("sensors_task started");
    for (;;) {
        SLOG_DEBUG("device scan begin");
        for (uint8_t addr = 1; addr < 128; addr++) {
            if (HAL_I2C_IsDeviceReady(&sens_i2c, (addr << 1), 3, 5) == HAL_OK) {
                SLOG_DEBUG("found device at 0x%02X", addr);
            }
        }
        SLOG_DEBUG("device scan end");
        osDelay(10000);
    }
}
