/**
 * @file datetime.h
 * @brief Functions to work with datetime
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include <stdint.h>

 /**
  * @brief Convert datatime to unix timestamp
  *
  * @return uint32_t timestamp
  */
uint32_t datetime_to_timestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
