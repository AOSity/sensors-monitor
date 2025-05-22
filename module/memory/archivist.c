/**
 * @file archivist.c
 * @brief Contains logics for processing (save, display) sensor readings
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "sensors.h"
#include "memory.h"
#include "gui.h"
#include "slog.h"
#include "cmsis_os2.h"
#include <stdbool.h>

#define SENSOR_READ_VALUE_PERIOD_S 60
#define CHART_PUSH_VALUE_PERIOD_S  300
#define MEMORY_SAVE_VALUE_PERIOD_S 900

static volatile bool need_sensor_read = true;
static volatile bool need_chart_push = true;
static volatile bool need_memory_save = false;

static int32_t last_data[SENSOR_TYPE_COUNT] = {0};
static int32_t chart_push_data[SENSOR_TYPE_COUNT] = {0};
static int32_t memory_save_data[SENSOR_TYPE_COUNT] = {0};
extern memory_driver_t memory;

static void reading_handler(sensor_data_type_t type, int32_t value) {
    last_data[type] = value;

    if (chart_push_data[type] == 0) {
        chart_push_data[type] = value;
    } else {
        chart_push_data[type] = value + (chart_push_data[type] - value) / 2;
    }

    memory_save_data[type] = chart_push_data[type];
}

static void sensor_read_periodic_cb(void* argument) {
    need_sensor_read = true;
}

static void chart_push_periodic_cb(void* argument) {
    need_chart_push = true;
}

static void memory_save_periodic_cb(void* argument) {
    need_memory_save = true;
}


void archivist_task(void* argument) {
    osDelay(200);
    gui_init();

    memory_init_driver();
    memory.init();
    SLOG_DEBUG("memory id: 0x%06X", memory.get_id());

    while (!gui_sensmon_screen_ready()) {
        gui_process();
        osDelay(5);
    }

    osTimerId_t sensor_read_periodic = osTimerNew(sensor_read_periodic_cb, osTimerPeriodic, NULL, NULL);
    osTimerStart(sensor_read_periodic, SENSOR_READ_VALUE_PERIOD_S * 1000);
    osTimerId_t chart_push_periodic = osTimerNew(chart_push_periodic_cb, osTimerPeriodic, NULL, NULL);
    osTimerStart(chart_push_periodic, CHART_PUSH_VALUE_PERIOD_S * 1000);
    osTimerId_t memory_save_periodic = osTimerNew(memory_save_periodic_cb, osTimerPeriodic, NULL, NULL);
    osTimerStart(memory_save_periodic, MEMORY_SAVE_VALUE_PERIOD_S * 1000);

    for (;;) {
        if (need_sensor_read) {
            need_sensor_read = false;
            sensor_process_reading(reading_handler);
            for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
                if (last_data[type] != 0) {
                    gui_sensmon_update_current_value(type, last_data[type]);
                    last_data[type] = 0;
                } else {
                    gui_sensmon_update_current_value(type, LV_CHART_POINT_NONE);
                }
            }
        }
        if (need_chart_push) {
            need_chart_push = false;
            for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
                if (chart_push_data[type] != 0) {
                    gui_sensmon_push_chart_value(type, chart_push_data[type]);
                    chart_push_data[type] = 0;
                } else {
                    gui_sensmon_push_chart_value(type, LV_CHART_POINT_NONE);
                }
            }
        }
        if (need_memory_save) {
            need_memory_save = false;
            SLOG_DEBUG("periodic data save to memory");
        }
        gui_process();
        osDelay(5);
    }
}
