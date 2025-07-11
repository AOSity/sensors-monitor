/**
 * @file sensors.c
 * @brief Contains task for sensors detection and processing
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "sensors.h"
#include "slog.h"
#include "i2c.h"
#include "cmsis_os2.h"

typedef void (*process_reading_func_t)(sensor_reading_handler_t);

typedef struct {
    uint8_t sensor_addr;
    process_reading_func_t process_reading;
} sensor_map_entry_t;

sensor_map_entry_t sensors_map[] = {
    {0x1A, ags02ma_process_reading},
    {0x38, aht20_process_reading},
    {0x77, bmp280_process_reading},
    /** add new sensors here */
};

const int sensors_count = sizeof(sensors_map) / sizeof(sensor_map_entry_t);

static void process_reading_by_sensor_addr(uint8_t addr, sensor_reading_handler_t reading_handler) {
    for (int i = 0; i < sensors_count; i++) {
        if (sensors_map[i].sensor_addr == addr) {
            sensors_map[i].process_reading(reading_handler);
            return;
        }
    }
    SLOG_WARN("reading not supported for device at 0x%02X", addr);
}

void sensor_process_reading(sensor_reading_handler_t reading_handler) {
    SLOG_DEBUG("sensors bus scan begin");
    for (uint8_t addr = 1; addr < 128; addr++) {
        if (HAL_I2C_IsDeviceReady(&sens_i2c, (addr << 1), 3, 5) == HAL_OK) {
            SLOG_DEBUG("sensor found at 0x%02X", addr);
            process_reading_by_sensor_addr(addr, reading_handler);
        }
    }
    SLOG_DEBUG("sensors bus scan end");
}
