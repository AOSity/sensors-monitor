/**
 * @file gui_history.c
 * @brief Screen for selecting datetime and viewing sensor history
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gui.h"
#include "datetime.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

static lv_obj_t* history_screen_main_container;
static lv_obj_t* date_dropdown;
static lv_obj_t* hour_dropdown;
static lv_obj_t* show_button;
static lv_obj_t* data_display_area_container;
static lv_obj_t* history_charts[SENSOR_TYPE_COUNT];
static lv_chart_series_t* history_chart_series[SENSOR_TYPE_COUNT];

static char date_options_str[HISTORY_MAX_DATE_OPTIONS * 12];
static RTC_DateTypeDef selected_dates[HISTORY_MAX_DATE_OPTIONS];

history_data_fetcher_t history_data_fetcher_func;

static bool screen_is_currently_active = false;

static void display_fetched_history_data(RTC_DateTypeDef date, uint8_t hour);

static void fill_date_dropdown(void) {
    RTC_DateTypeDef current_rtc_date;
    RTC_TimeTypeDef dummy_time;

    if (HAL_RTC_GetTime(&hrtc, &dummy_time, RTC_FORMAT_BCD) != HAL_OK) {
        return;
    }
    if (HAL_RTC_GetDate(&hrtc, &current_rtc_date, RTC_FORMAT_BCD) != HAL_OK) {
        return;
    }

    RTC_DateTypeDef date_iter = current_rtc_date;
    date_options_str[0] = '\0';
    for (int i = 0; i < HISTORY_MAX_DATE_OPTIONS; i++) {
        selected_dates[i] = date_iter;

        char date_buf[12];
        snprintf(date_buf, sizeof(date_buf), "%02d.%02d.%02d", date_iter.Date, date_iter.Month, date_iter.Year);
        if (i > 0) {
            strcat(date_options_str, "\n");
        }
        strcat(date_options_str, date_buf);

        if (date_iter.Date > 1) {
            date_iter.Date--;
        } else {
            if (date_iter.Month > 1) {
                date_iter.Month--;
            } else {
                date_iter.Month = 12;
                if (date_iter.Year > 0) {
                    date_iter.Year--;
                } else {
                    date_iter.Year = 99;
                }
            }
            date_iter.Date = datetime_get_days_in_month(date_iter.Month);
            if (datetime_is_leap_year(2000 + date_iter.Year)) {
                date_iter.Date += 1;
            }
        }
    }
    lv_dropdown_set_options(date_dropdown, date_options_str);
    lv_dropdown_set_selected(date_dropdown, 0);
}

static void fill_hour_dropdown(void) {
    char hour_options_str[24 * 4];
    hour_options_str[0] = '\0';
    for (int i = 0; i < 24; i++) {
        char hour_buf[4];
        snprintf(hour_buf, sizeof(hour_buf), "%02d", i);
        if (i > 0) {
            strcat(hour_options_str, "\n");
        }
        strcat(hour_options_str, hour_buf);
    }
    lv_dropdown_set_options(hour_dropdown, hour_options_str);
    lv_dropdown_set_selected(hour_dropdown, 12);
}

static void show_button_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        uint32_t date_idx = lv_dropdown_get_selected(date_dropdown);
        uint32_t hour = lv_dropdown_get_selected(hour_dropdown);
        RTC_DateTypeDef chosen_date_bcd = selected_dates[date_idx];
        if (data_display_area_container) {
            lv_obj_clean(data_display_area_container);
        }
        for (uint8_t i = 0; i < SENSOR_TYPE_COUNT; ++i) {
            history_charts[i] = NULL;
            history_chart_series[i] = NULL;
        }
        display_fetched_history_data(chosen_date_bcd, (uint8_t)hour);
    }
}

static void display_fetched_history_data(RTC_DateTypeDef date_bcd, uint8_t hour) {
    if (!history_data_fetcher_func || !data_display_area_container) {
        lv_obj_t* temp_label = lv_label_create(data_display_area_container);
        lv_label_set_text_fmt(temp_label,
            "History fetcher not available or UI error.\nSelected: %02x.%02x.20%02x, %02d:00", date_bcd.Date, date_bcd.Month, date_bcd.Year, hour);
        lv_obj_center(temp_label);
        return;
    }

    uint32_t timestamp_hour_start = datetime_to_timestamp(2000 + date_bcd.Year, date_bcd.Month, date_bcd.Date, hour, 0, 0);
    int32_t fetched_values[HISTORY_CHART_POINTS];
    for (sensor_data_type_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
        history_data_fetcher_func(type, timestamp_hour_start, fetched_values, HISTORY_CHART_POINTS);

        lv_obj_t* chart_block = lv_obj_create(data_display_area_container);
        lv_obj_remove_style_all(chart_block);
        lv_obj_set_width(chart_block, lv_pct(100));
        lv_obj_set_height(chart_block, 60);
        lv_obj_set_layout(chart_block, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(chart_block, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_all(chart_block, 3, 0);
        lv_obj_set_style_bg_color(chart_block, lv_color_white(), 0);
        lv_obj_set_style_radius(chart_block, 3, 0);
        lv_obj_set_style_margin_bottom(chart_block, 5, 0);
        lv_obj_set_style_shadow_opa(chart_block, LV_OPA_30, 0);
        lv_obj_set_style_shadow_width(chart_block, 5, 0);
        lv_obj_set_style_shadow_offset_y(chart_block, 2, 0);

        lv_obj_t* name_label_cont = lv_obj_create(chart_block);
        lv_obj_remove_style_all(name_label_cont);
        lv_obj_set_width(name_label_cont, 100);
        lv_obj_set_height(name_label_cont, lv_pct(100));
        lv_obj_set_style_pad_all(name_label_cont, 5, 0);

        lv_obj_t* name_label = lv_label_create(name_label_cont);
        switch (type) {
            case SENSOR_TEMPERATURE:
                lv_label_set_text(name_label, "Temperature");
                break;
            case SENSOR_HUMIDITY:
                lv_label_set_text(name_label, "Humidity");
                break;
            case SENSOR_PRESSURE:
                lv_label_set_text(name_label, "Pressure");
                break;
            case SENSOR_TVOC:
                lv_label_set_text(name_label, "TVOC");
                break;
            default:
                lv_label_set_text(name_label, "N/A");
                break;
        }
        lv_obj_set_style_text_font(name_label, &lv_font_montserrat_14, 0);
        lv_obj_center(name_label);

        history_charts[type] = lv_chart_create(chart_block);
        lv_obj_set_style_flex_grow(history_charts[type], 1, 0);
        lv_obj_set_height(history_charts[type], lv_pct(100));
        lv_chart_set_type(history_charts[type], LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(history_charts[type], HISTORY_CHART_POINTS);
        lv_chart_set_update_mode(history_charts[type], LV_CHART_UPDATE_MODE_SHIFT);
        lv_chart_set_div_line_count(history_charts[type], 3, 5);
        lv_obj_set_style_bg_color(history_charts[type], lv_color_hex(0xF0F0F0), 0);

        history_chart_series[type] = lv_chart_add_series(history_charts[type], lv_palette_main(LV_PALETTE_BLUE_GREY), LV_CHART_AXIS_PRIMARY_Y);

        int32_t min_y = LV_COORD_MAX, max_y = LV_COORD_MIN;
        for (uint16_t i = 0; i < HISTORY_CHART_POINTS; i++) {
            lv_chart_set_next_value(history_charts[type], history_chart_series[type], fetched_values[i]);
            if (fetched_values[i] == LV_CHART_POINT_NONE) {
                continue;
            }
            if (fetched_values[i] < min_y) min_y = fetched_values[i];
            if (fetched_values[i] > max_y) max_y = fetched_values[i];
        }

        lv_chart_set_range(history_charts[type], LV_CHART_AXIS_PRIMARY_Y, (lv_coord_t)min_y, (lv_coord_t)max_y);
        lv_chart_refresh(history_charts[type]);
    }
}


void gui_history_screen_create(lv_obj_t* parent_of_screens_base_layer) {
    if (screen_is_currently_active) {
        return;
    }

    history_screen_main_container = lv_obj_create(parent_of_screens_base_layer);
    lv_obj_remove_style_all(history_screen_main_container);
    lv_obj_set_size(history_screen_main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(history_screen_main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(history_screen_main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(history_screen_main_container, 10, 0);
    lv_obj_set_style_pad_row(history_screen_main_container, 10, 0);

    lv_obj_t* controls_cont = lv_obj_create(history_screen_main_container);
    lv_obj_remove_style_all(controls_cont);
    lv_obj_set_width(controls_cont, lv_pct(100));
    lv_obj_set_height(controls_cont, LV_SIZE_CONTENT);
    lv_obj_set_layout(controls_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(controls_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(controls_cont, 5, 0);

    lv_obj_t* date_group = lv_obj_create(controls_cont);
    lv_obj_remove_style_all(date_group);
    lv_obj_set_layout(date_group, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(date_group, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(date_group, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_height(date_group, LV_SIZE_CONTENT);
    lv_obj_set_width(date_group, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_column(date_group, 5, 0);

    lv_obj_t* date_label = lv_label_create(date_group);
    lv_label_set_text(date_label, "Date:");
    date_dropdown = lv_dropdown_create(date_group);
    lv_obj_set_width(date_dropdown, 130);
    fill_date_dropdown();

    lv_obj_t* hour_group = lv_obj_create(controls_cont);
    lv_obj_remove_style_all(hour_group);
    lv_obj_set_layout(hour_group, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hour_group, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hour_group, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_height(hour_group, LV_SIZE_CONTENT);
    lv_obj_set_width(hour_group, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_column(hour_group, 5, 0);

    lv_obj_t* hour_label = lv_label_create(hour_group);
    lv_label_set_text(hour_label, "Hour:");
    hour_dropdown = lv_dropdown_create(hour_group);
    lv_obj_set_width(hour_dropdown, 70);
    fill_hour_dropdown();

    show_button = lv_btn_create(controls_cont);
    lv_obj_t* btn_label = lv_label_create(show_button);
    lv_label_set_text(btn_label, "Display");
    lv_obj_center(btn_label);
    lv_obj_add_event_cb(show_button, show_button_event_cb, LV_EVENT_CLICKED, NULL);

    data_display_area_container = lv_obj_create(history_screen_main_container);
    lv_obj_remove_style_all(data_display_area_container);
    lv_obj_set_width(data_display_area_container, lv_pct(100));
    lv_obj_set_style_flex_grow(data_display_area_container, 1, 0);
    lv_obj_set_layout(data_display_area_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(data_display_area_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(data_display_area_container, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(data_display_area_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(data_display_area_container, 5, 0);
    lv_obj_set_style_pad_row(data_display_area_container, 5, 0);

    lv_obj_t* initial_msg_label = lv_label_create(data_display_area_container);
    lv_label_set_text(initial_msg_label, "Select date and hour, then click 'Display'.");
    lv_obj_align(initial_msg_label, LV_ALIGN_CENTER, 0, 0);

    for (int i = 0; i < SENSOR_TYPE_COUNT; ++i) {
        history_charts[i] = NULL;
        history_chart_series[i] = NULL;
    }

    screen_is_currently_active = true;
}

void gui_history_screen_destroy(void) {
    if (!screen_is_currently_active) {
        return;
    }
    if (history_screen_main_container) {
        lv_obj_del(history_screen_main_container);
        history_screen_main_container = NULL;
    }
    date_dropdown = NULL;
    hour_dropdown = NULL;
    show_button = NULL;
    data_display_area_container = NULL;
    for (int i = 0; i < SENSOR_TYPE_COUNT; ++i) {
        history_charts[i] = NULL;
        history_chart_series[i] = NULL;
    }
    screen_is_currently_active = false;
}

bool gui_history_screen_is_active(void) {
    return screen_is_currently_active;
}

void gui_history_init_data_fetcher(history_data_fetcher_t data_fetcher_func) {
    history_data_fetcher_func = data_fetcher_func;
}
