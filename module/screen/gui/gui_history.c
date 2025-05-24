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
static lv_obj_t* data_display_area;

static char date_options_str[HISTORY_MAX_DATE_OPTIONS * 12];
static RTC_DateTypeDef selected_dates[HISTORY_MAX_DATE_OPTIONS];

static bool screen_is_currently_active = false;

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
        int date_idx = lv_dropdown_get_selected(date_dropdown);
        int hour = lv_dropdown_get_selected(hour_dropdown);

        RTC_DateTypeDef chosen_date = selected_dates[date_idx];

        char display_text[128];
        snprintf(display_text, sizeof(display_text),
            "Fetching history for: %02d.%02d.20%02d, Hour: %02d:00\n(Implement data retrieval logic here)",
            chosen_date.Date, chosen_date.Month, chosen_date.Year, hour);

        if (data_display_area) {
            lv_label_set_text(data_display_area, display_text);
        }
        LV_LOG_USER("History: Date %02d.%02d.%02d, Hour %02d", chosen_date.Date, chosen_date.Month, chosen_date.Year, hour);

        /* TODO: Implement data fetching and display logic */
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

    data_display_area = lv_label_create(history_screen_main_container);
    lv_obj_set_width(data_display_area, lv_pct(100));
    lv_obj_set_style_flex_grow(data_display_area, 1, 0);

    lv_label_set_long_mode(data_display_area, LV_LABEL_LONG_WRAP);
    lv_label_set_text(data_display_area, "Select date and hour, then click 'Display'.");
    lv_obj_set_style_text_align(data_display_area, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_all(data_display_area, 5, 0);
    lv_obj_set_style_bg_color(data_display_area, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(data_display_area, 1, 0);
    lv_obj_set_style_border_color(data_display_area, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_radius(data_display_area, 3, 0);

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
    data_display_area = NULL;
    screen_is_currently_active = false;
}

bool gui_history_screen_is_active(void) {
    return screen_is_currently_active;
}
