/**
 * @file aht20.c
 * @brief AHT20 temperature & humidity reading processing
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "sensors.h"
#include "slog.h"
#include "i2c.h"
#include "cmsis_os2.h"

#define AHT20_ADDR (0x38 << 1)
#define AGS02MA_CMD_GET_READING 0xAC

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

void aht20_process_reading(void) {
    uint8_t rx_buffer[7];
    uint8_t cmd[3] = { AGS02MA_CMD_GET_READING, 0x33, 0x00 };

    if (HAL_I2C_Master_Transmit(&sens_i2c, AHT20_ADDR, cmd, 3, HAL_MAX_DELAY) != HAL_OK) {
        SLOG_ERROR("AHT20 data request failed");
        return;
    }

    osDelay(100);

    if (HAL_I2C_Master_Receive(&sens_i2c, AHT20_ADDR, rx_buffer, 7, HAL_MAX_DELAY) != HAL_OK) {
        SLOG_ERROR("AHT20 data receive failed");
        return;
    }

    if (calc_crc8(rx_buffer, 6) != rx_buffer[6]) {
        SLOG_ERROR("AGS02MA data receive failed, CRC8 mismatch");
        return;
    }

    if ((rx_buffer[0] & 0x80) != 0) {
        SLOG_WARN("AHT20 not ready");
        return;
    }

    uint32_t raw_hum = ((uint32_t)(rx_buffer[1]) << 12) | ((uint32_t)(rx_buffer[2]) << 4) | (rx_buffer[3] >> 4);
    uint32_t raw_temp = (((uint32_t)(rx_buffer[3] & 0x0F)) << 16) | ((uint32_t)(rx_buffer[4]) << 8) | rx_buffer[5];
    uint32_t humidity = (raw_hum * 1000) / 1048576;
    int32_t temperature = ((int32_t)raw_temp * 2000 / 1048576) - 500;

    SLOG_DEBUG("AHT20: Temperature = %d.%02d °C, Humidity = %u.%02u %%", temperature / 10, temperature % 10, humidity / 10, humidity % 10);
}
