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

extern memory_driver_t memory;
static int32_t reading[SENSOR_TYPE_COUNT] = {0};

static void reading_handler(sensor_data_type_t type, int32_t value) {
    if (reading[type] == 0) {
        reading[type] = value;
    } else {
        reading[type] = value + (reading[type] - value) / 2;
    }
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

    for (;;) {
        sensor_process_reading(reading_handler);
        for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
            if (reading[type] == 0) {
                continue;
            }
            gui_sensmon_push_chart_value(type, reading[type]);
            gui_sensmon_update_current_value(type, reading[type]);
            reading[type] = 0;
        }
        for (uint16_t i = 0; i < 1000; i++) {
            gui_process();
            osDelay(5);
        }
    }
}
