/**
 * @file gui.c
 * @brief Graphical user interface api
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#pragma once

#include "sensors.h"
#include <lvgl.h>
#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define SENSOR_MONITOR_MAX_CHARTS 4
#define SENSOR_MONITOR_MAX_POINTS 12

#define HISTORY_MAX_DATE_OPTIONS 7
#define HISTORY_CHART_POINTS 6

typedef enum {
    GUI_SCREEN_SENSORS,
    GUI_SCREEN_HISTORY,
    GUI_SCREEN_COUNT,
} gui_screen_id_t;

typedef void (*history_data_fetcher_t)(sensor_data_type_t, uint32_t, int32_t*, uint16_t);

void gui_init(void);
void gui_process(void);

void gui_datetime_screen_init(lv_obj_t* parent);
bool gui_is_datetime_configured(void);

void gui_manager_init(void);
void gui_switch_screen(gui_screen_id_t screen_id);

void gui_sensmon_data_init(void);
void gui_sensmon_screen_create(lv_obj_t* parent);
bool gui_sensmon_screen_is_built(void);
void gui_sensmon_screen_set_visibility(bool visible);
void gui_sensmon_update_current_value(sensor_data_type_t type, int32_t value);
void gui_sensmon_push_chart_value(sensor_data_type_t type, int32_t value);

void gui_history_screen_create(lv_obj_t* parent);
void gui_history_screen_destroy(void);
bool gui_history_screen_is_active(void);
void gui_history_init_data_fetcher(history_data_fetcher_t);
