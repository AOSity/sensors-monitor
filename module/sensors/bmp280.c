/**
 * @file bmp280.c
 * @brief BMP280 pressure sensor reading processing
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "sensors.h"
#include "slog.h"
#include "i2c.h"
#include "cmsis_os2.h"

#define BMP280_ADDR            (0x77 << 1)
#define BMP280_REG_ID          0xD0
#define BMP280_REG_CALIB_START 0x88
#define BMP280_REG_CTRL_MEAS   0xF4
#define BMP280_REG_PRESS_MSB   0xF7
#define BMP280_REG_TEMP_MSB    0xFA

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_data;

static bmp280_calib_data calib;
static int32_t t_fine = 0;

static HAL_StatusTypeDef bmp280_read_bytes(uint8_t reg, uint8_t* buf, uint16_t len) {
    return HAL_I2C_Mem_Read(&sens_i2c, BMP280_ADDR, reg, 1, buf, len, HAL_MAX_DELAY);
}

static void bmp280_read_calibration(void) {
    uint8_t calib_data[24];
    if (bmp280_read_bytes(BMP280_REG_CALIB_START, calib_data, 24) != HAL_OK) {
        SLOG_ERROR("BMP280 failed to read calibration data");
        return;
    }

    calib.dig_T1 = (calib_data[1] << 8) | calib_data[0];
    calib.dig_T2 = (int16_t)((calib_data[3] << 8) | calib_data[2]);
    calib.dig_T3 = (int16_t)((calib_data[5] << 8) | calib_data[4]);
    calib.dig_P1 = (calib_data[7] << 8) | calib_data[6];
    calib.dig_P2 = (int16_t)((calib_data[9] << 8) | calib_data[8]);
    calib.dig_P3 = (int16_t)((calib_data[11] << 8) | calib_data[10]);
    calib.dig_P4 = (int16_t)((calib_data[13] << 8) | calib_data[12]);
    calib.dig_P5 = (int16_t)((calib_data[15] << 8) | calib_data[14]);
    calib.dig_P6 = (int16_t)((calib_data[17] << 8) | calib_data[16]);
    calib.dig_P7 = (int16_t)((calib_data[19] << 8) | calib_data[18]);
    calib.dig_P8 = (int16_t)((calib_data[21] << 8) | calib_data[20]);
    calib.dig_P9 = (int16_t)((calib_data[23] << 8) | calib_data[22]);
}

static int32_t bmp280_compensate_temperature(int32_t adc_t) {
    int32_t var1 = ((((adc_t >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;
    int32_t var2 = (((((adc_t >> 4) - ((int32_t)calib.dig_T1)) * ((adc_t >> 4) - ((int32_t)calib.dig_T1))) >> 12) * ((int32_t)calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

static uint32_t bmp280_compensate_pressure(int32_t adc_p) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * ((int64_t)calib.dig_P1)) >> 33;

    if (var1 == 0) {
        /* avoid division by zero */
        return 0;
    }

    p = 1048576 - adc_p;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    return (uint32_t)(p >> 8);
}

void bmp280_process_reading(sensor_reading_handler_t reading_handler) {
    uint8_t id;
    if (bmp280_read_bytes(BMP280_REG_ID, &id, 1) != HAL_OK || id != 0x58) {
        SLOG_ERROR("BMP280 not found or wrong ID: 0x%02X", id);
        return;
    }

    bmp280_read_calibration();

    uint8_t ctrl_meas = 0x27;
    HAL_I2C_Mem_Write(&sens_i2c, BMP280_ADDR, BMP280_REG_CTRL_MEAS, 1, &ctrl_meas, 1, HAL_MAX_DELAY);

    osDelay(100);

    uint8_t data[6];
    if (bmp280_read_bytes(BMP280_REG_PRESS_MSB, data, 6) != HAL_OK) {
        SLOG_ERROR("BMP280 read failed");
        return;
    }

    int32_t adc_t = (int32_t)(((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4));
    int32_t adc_p = (int32_t)(((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4));
    int32_t temperature = bmp280_compensate_temperature(adc_t);
    uint32_t pressure = bmp280_compensate_pressure(adc_p);

    SLOG_DEBUG("BMP280: Temperature = %d.%02d °C, Pressure = %lu hPa", temperature / 100, temperature % 100, pressure / 100);

    if (reading_handler) {
        (*reading_handler)(SENSOR_TEMPERATURE, (int32_t)temperature);
        (*reading_handler)(SENSOR_PRESSURE, (int32_t)pressure);
    }
}