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

#define SENSOR_MONITOR_MAX_CHARTS 4
#define SENSOR_MONITOR_MAX_POINTS 12

void gui_init(void);
void gui_process(void);

void gui_sensmon_create_screen(lv_obj_t* parent);
void gui_sensmon_create_chart(sensor_data_type_t type);
void gui_sensmon_update_current_value(sensor_data_type_t type, int32_t value);
void gui_sensmon_push_chart_value(sensor_data_type_t type, int32_t value);
