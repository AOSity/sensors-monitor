/**
 * @file datetime.h
 * @brief Functions to work with datetime
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Get number of days in month
 *
 * @return uint8_t number of days in month
 */
uint8_t datetime_get_days_in_month(uint8_t month);

/**
 * @brief Check if provided year is leap
 *
 * @return true - leap year, false otherwise
 */
bool datetime_is_leap_year(uint16_t year);

/**
 * @brief Convert datatime to unix timestamp
 *
 * @return uint32_t timestamp
 */
uint32_t datetime_to_timestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
