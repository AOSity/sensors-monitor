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
    SENSOR_UNKNOWN = 0,
    SENSOR_TEMPERATURE,
    SENSOR_HUMIDITY,
    SENSOR_PRESSURE,
    SENSOR_TVOC,
} sensor_data_type_t;

void ags02ma_process_reading(void);
void bmp280_process_reading(void);
void aht20_process_reading(void);
