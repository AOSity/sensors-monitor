/**
 * @file gui.c
 * @brief Sensors Monitor screen gui
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gui.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

static volatile bool is_screen_initialized = false;

static lv_obj_t* main_container;
static lv_obj_t* title_label;

static lv_obj_t* roller_day;
static lv_obj_t* roller_month;
static lv_obj_t* roller_year;
static lv_obj_t* roller_hour;
static lv_obj_t* roller_minute;

static void save_btn_event_cb(lv_event_t* e) {
    uint8_t day = lv_roller_get_selected(roller_day) + 1;
    uint8_t month = lv_roller_get_selected(roller_month) + 1;
    uint8_t year = lv_roller_get_selected(roller_year);
    RTC_DateTypeDef date = {.Date = day, .Month = month, .Year = year};
    uint8_t hour = lv_roller_get_selected(roller_hour);
    uint8_t minute = lv_roller_get_selected(roller_minute);
    RTC_TimeTypeDef time = {.Hours = hour, .Minutes = minute};
    HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);
    HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD);
    gui_datetime_screen_deinit();
    gui_sensmon_screen_init(lv_scr_act());
}

static lv_obj_t* create_labeled_roller(lv_obj_t* parent, const char* label_text, uint16_t min_value, uint16_t max_value) {
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_pad_row(container, 2, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);

    lv_obj_t* label = lv_label_create(container);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label, 60);

    lv_obj_t* roller = lv_roller_create(container);
    char options[512] = "";
    for (uint16_t i = min_value; i <= max_value; i++) {
        char buf[7];
        snprintf(buf, sizeof(buf), "%02u\n", i);
        strcat(options, buf);
    }
    lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(roller, 3);
    lv_obj_set_width(roller, 60);

    return roller;
}

void gui_datetime_screen_init(lv_obj_t* parent) {
    main_container = lv_obj_create(parent);
    lv_obj_set_size(main_container, 480, 320);
    lv_obj_center(main_container);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(main_container, LV_DIR_NONE);
    lv_obj_set_style_pad_all(main_container, 8, 0);
    lv_obj_set_style_pad_row(main_container, 12, 0);

    title_label = lv_label_create(main_container);
    lv_label_set_text(title_label, "Configure Date & Time");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t* content_container = lv_obj_create(main_container);
    lv_obj_set_size(content_container, lv_pct(100), 240);
    lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(content_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(content_container, 0, 0);
    lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_row(content_container, 20, 0);

    lv_obj_t* roller_row = lv_obj_create(content_container);
    lv_obj_set_width(roller_row, lv_pct(100));
    lv_obj_set_height(roller_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(roller_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(roller_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(roller_row, 10, 0);
    lv_obj_clear_flag(roller_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(roller_row, 0, 0);
    lv_obj_set_style_bg_opa(roller_row, LV_OPA_TRANSP, 0);

    roller_day = create_labeled_roller(roller_row, "Day", 1, 31);
    lv_roller_set_selected(roller_day, 14, LV_ANIM_OFF);
    roller_month = create_labeled_roller(roller_row, "Month", 1, 12);
    lv_roller_set_selected(roller_month, 4, LV_ANIM_OFF);
    roller_year = create_labeled_roller(roller_row, "Year", 2000, 2099);
    lv_roller_set_selected(roller_year, 25, LV_ANIM_OFF);
    roller_hour = create_labeled_roller(roller_row, "Hour", 0, 23);
    lv_roller_set_selected(roller_hour, 12, LV_ANIM_OFF);
    roller_minute = create_labeled_roller(roller_row, "Minute", 0, 59);
    lv_roller_set_selected(roller_minute, 30, LV_ANIM_OFF);

    lv_obj_t* btn_save = lv_btn_create(content_container);
    lv_obj_set_width(btn_save, 100);
    lv_obj_center(btn_save);

    lv_obj_t* label = lv_label_create(btn_save);
    lv_label_set_text(label, "Save");
    lv_obj_center(label);
    lv_obj_add_event_cb(btn_save, save_btn_event_cb, LV_EVENT_CLICKED, NULL);
    is_screen_initialized = true;
}

void gui_datetime_screen_deinit(void) {
    is_screen_initialized = false;
    lv_obj_delete(main_container);
    main_container = NULL;
    title_label = NULL;
    roller_day = NULL;
    roller_month = NULL;
    roller_year = NULL;
    roller_hour = NULL;
    roller_minute = NULL;
}

bool gui_datetime_screen_ready(void) {
    return is_screen_initialized;
}
