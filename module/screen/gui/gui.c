/**
 * @file gui.c
 * @brief Graphical user interface functions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gui.h"
#include "screen.h"
#include "indev.h"
#include "slog.h"
#include "main.h"
#include "cmsis_os2.h"

static uint8_t buf1[UINT16_MAX];
static uint8_t buf2[UINT16_MAX];
extern screen_driver_t screen;
extern indev_t indev;

/**
 * @brief Callback used to send rendered data to screen
 *
 * @param display lv display handle
 * @param area    lv display area
 * @param px_map  pixels
 */
static void lv_flush_cb(lv_display_t* display, const lv_area_t* area, uint8_t* px_map) {
    screen.set_window(area->x1, area->y1, area->x2, area->y2);
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;
    size_t size = w * h;
    uint16_t* buf16 = (uint16_t*)px_map;
    screen.send_pixels(buf16, size);
    lv_display_flush_ready(display);
}

/**
 * @brief Callback used to get input data
 *
 * @param input input device
 * @param data input device data
 */
static void lv_input_read(lv_indev_t* input, lv_indev_data_t* data) {
    switch (indev.type) {
        case INDEV_TYPE_TOUCH:
            indev.touch.read();
            if (indev.touch.touched) {
                data->state = LV_INDEV_STATE_PRESSED;
                data->point.x = indev.touch.x;
                data->point.y = indev.touch.y;
            } else {
                data->state = LV_INDEV_STATE_RELEASED;
            }
            return;
        default:
            return;
    }
}

void gui_init(void) {
    screen_init_driver();
    SLOG_DEBUG("gui init screen driver");
    screen.init();
    HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET);
    SLOG_DEBUG("gui init screen");
    indev_init();
    SLOG_DEBUG("gui init input device");

    lv_init();
    lv_tick_set_cb(osKernelGetTickCount);
    lv_display_t* display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_buffers(display, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, lv_flush_cb);

    lv_indev_t* input = lv_indev_create();
    switch (indev.type) {
        case INDEV_TYPE_TOUCH:
            lv_indev_set_type(input, LV_INDEV_TYPE_POINTER);
            break;
        default:
            SLOG_ERROR("unsupported indev type");
            break;
    }
    lv_indev_set_read_cb(input, lv_input_read);
    SLOG_DEBUG("gui init lvgl");

    gui_sensmon_data_init();
    gui_datetime_screen_init(lv_scr_act());
}

void gui_process(void) {
    lv_timer_handler();
}
