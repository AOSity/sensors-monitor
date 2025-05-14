/**
 * @file sensors.h
 * @brief Sensors structures and definitions
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#pragma once

#include <stdint.h>

 /**
  * @brief Sensors measurements data types
  */
typedef enum {
    SENSOR_TEMPERATURE,
    SENSOR_HUMIDITY,
    SENSOR_PRESSURE,
    SENSOR_TVOC,
    SENSOR_TYPE_COUNT,
} sensor_data_type_t;

typedef void (*sensor_reading_handler_t)(sensor_data_type_t, int32_t);

void ags02ma_process_reading(sensor_reading_handler_t);
void bmp280_process_reading(sensor_reading_handler_t);
void aht20_process_reading(sensor_reading_handler_t);
