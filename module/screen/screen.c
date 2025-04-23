/**
 * @file screen.c
 * @brief Contains task for screen initialization and graphics
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "screen.h"
#include "slog.h"
#include "main.h"
#include "cmsis_os2.h"

extern screen_driver_t screen;

void screen_task(void* argument) {
    osDelay(150);
    SLOG_DEBUG("screen_task started");
    screen_init_driver();
    SLOG_DEBUG("screen_task init driver");
    screen.init();
    SLOG_DEBUG("screen_task init screen");
    osDelay(150);

    HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET);
    for (;;) {
        osDelay(1000);
    }
}
