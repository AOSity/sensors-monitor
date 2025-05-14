/**
 * @file ags02ma.c
 * @brief AGS02MA TVOC sensor reading processing
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "sensors.h"
#include "slog.h"
#include "i2c.h"
#include "cmsis_os2.h"

#define AGS02MA_ADDR (0x1A << 1)
#define AGS02MA_CMD_GET_READING 0x00

/**
 * @brief CRC8 calculation for received data
 *
 * @param data received data buffer
 * @param size size of data
 * @return uint8_t crc
 */
static uint8_t calc_crc8(const uint8_t* data, uint8_t size) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < size; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void ags02ma_process_reading(sensor_reading_handler_t reading_handler) {
    uint8_t rx_buffer[5];
    uint8_t cmd = AGS02MA_CMD_GET_READING;

    if (HAL_I2C_Master_Transmit(&sens_i2c, AGS02MA_ADDR, &cmd, 1, HAL_MAX_DELAY) != HAL_OK) {
        SLOG_ERROR("AGS02MA data request failed");
        return;
    }

    osDelay(100);

    if (HAL_I2C_Master_Receive(&sens_i2c, AGS02MA_ADDR, rx_buffer, 5, HAL_MAX_DELAY) != HAL_OK) {
        SLOG_ERROR("AGS02MA data receive failed");
        return;
    }

    if (calc_crc8(rx_buffer, 4) != rx_buffer[4]) {
        SLOG_ERROR("AGS02MA data receive failed, CRC8 mismatch");
        return;
    }

    uint32_t raw_value = (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
    SLOG_DEBUG("AGS02MA: TVOC = %lu ppb", raw_value);
}
