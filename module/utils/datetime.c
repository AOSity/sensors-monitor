/**
 * @file datetime.c
 * @brief Functions to work with datetime
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "datetime.h"
#include <stdbool.h>

uint8_t datetime_get_days_in_month(uint8_t month) {
    const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 0 || month > 12) {
        return 0;
    }
    return days_in_month[month];
}

bool datetime_is_leap_year(uint16_t year) {
    return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

uint32_t datetime_to_timestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    uint32_t days = 0;

    /* Add days for previous years */
    for (int y = 1970; y < year; y++) {
        days += datetime_is_leap_year(y) ? 366 : 365;
    }

    /* Add days for previous months in current year */
    for (int m = 1; m < month; m++) {
        days += datetime_get_days_in_month(m - 1);
        if (m == 2 && datetime_is_leap_year(year)) {
            days += 1;
        }
    }

    /* Add days in current month */
    days += (day - 1);

    /* Convert datetime to seconds */
    uint32_t timestamp = days * 86400 + hour * 3600 + min * 60 + sec;

    return timestamp;
}
