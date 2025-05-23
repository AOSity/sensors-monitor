/**
 * @file gui.c
 * @brief Sensors Monitor screen gui
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gui.h"
#include <stdio.h>

static volatile bool is_screen_initialized = false;

typedef struct {
    lv_obj_t* container;
    lv_obj_t* label_container;
    lv_obj_t* label;
    lv_obj_t* curr_value;
    lv_obj_t* chart;
    lv_chart_series_t* series;
    int32_t values[SENSOR_MONITOR_MAX_POINTS];
    uint8_t count;
    bool initialized;
} sensor_chart_t;

static lv_obj_t* main_container;
static lv_obj_t* title_label;
static sensor_chart_t charts[SENSOR_MONITOR_MAX_CHARTS];
static uint8_t chart_count = 0;

void gui_sensmon_screen_init(lv_obj_t* parent) {
    main_container = lv_obj_create(parent);
    lv_obj_set_size(main_container, 480, 320);
    lv_obj_center(main_container);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(main_container, LV_DIR_NONE);
    lv_obj_set_style_pad_all(main_container, 4, 0);
    lv_obj_set_style_pad_row(main_container, 4, 0);

    title_label = lv_label_create(main_container);
    lv_label_set_text(title_label, "Sensors Monitor");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 4, 4);
    is_screen_initialized = true;
}

void gui_sensmon_screen_deinit(void) {
    is_screen_initialized = false;
    lv_obj_del(main_container);
    main_container = NULL;
    title_label = NULL;
    lv_memset(charts, 0, sizeof(charts));
    chart_count = 0;
}

bool gui_sensmon_screen_ready(void) {
    return is_screen_initialized;
}


void gui_sensmon_create_chart(sensor_data_type_t type) {
    if (type >= SENSOR_MONITOR_MAX_CHARTS || charts[type].initialized || !is_screen_initialized) {
        return;
    }

    sensor_chart_t* sc = &charts[type];

    int available_height = 320 - 30 - (SENSOR_MONITOR_MAX_CHARTS - 1) * 4;
    int chart_height = available_height / (chart_count + 1);

    for (int i = 0; i < SENSOR_MONITOR_MAX_CHARTS; i++) {
        if (charts[i].initialized) {
            lv_obj_set_height(charts[i].container, chart_height);
            lv_obj_set_size(charts[i].chart, 320, chart_height - 12);
            lv_obj_set_size(charts[i].label_container, 120, chart_height - 12);
        }
    }

    sc->container = lv_obj_create(main_container);
    lv_obj_set_size(sc->container, 460, chart_height);
    lv_obj_set_flex_flow(sc->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_scroll_dir(sc->container, LV_DIR_NONE);
    lv_obj_set_style_pad_column(sc->container, 8, 0);
    lv_obj_set_style_pad_all(sc->container, 4, 0);

    sc->label_container = lv_obj_create(sc->container);
    lv_obj_set_size(sc->label_container, 120, chart_height - 12);
    lv_obj_set_scroll_dir(sc->label_container, LV_DIR_NONE);

    sc->label = lv_label_create(sc->label_container);
    switch (type) {
        case SENSOR_TEMPERATURE:
            lv_label_set_text(sc->label, "Temperature");
            break;
        case SENSOR_HUMIDITY:
            lv_label_set_text(sc->label, "Humidity");
            break;
        case SENSOR_PRESSURE:
            lv_label_set_text(sc->label, "Pressure");
            break;
        case SENSOR_TVOC:
            lv_label_set_text(sc->label, "TVOC");
            break;
        default:
            return;
    }
    lv_obj_set_width(sc->label, LV_SIZE_CONTENT);
    lv_obj_align(sc->label, LV_ALIGN_CENTER, 0, -10);
    lv_label_set_long_mode(sc->label, LV_LABEL_LONG_WRAP);

    sc->curr_value = lv_label_create(sc->label_container);
    lv_obj_set_width(sc->curr_value, LV_SIZE_CONTENT);
    lv_obj_align(sc->curr_value, LV_ALIGN_CENTER, 0, 10);
    lv_label_set_long_mode(sc->curr_value, LV_LABEL_LONG_WRAP);

    sc->chart = lv_chart_create(sc->container);
    lv_obj_set_size(sc->chart, 320, chart_height - 12);
    lv_chart_set_type(sc->chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(sc->chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_point_count(sc->chart, SENSOR_MONITOR_MAX_POINTS);
    lv_chart_set_update_mode(sc->chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(sc->chart, 4, 12);

    sc->series = lv_chart_add_series(sc->chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    for (int i = 0; i < SENSOR_MONITOR_MAX_POINTS; i++) {
        sc->values[i] = 0;
        lv_chart_set_value_by_id(sc->chart, sc->series, i, LV_CHART_POINT_NONE);
    }

    lv_chart_refresh(sc->chart);

    sc->count = 0;
    sc->initialized = true;
    chart_count++;
}

void gui_sensmon_update_current_value(sensor_data_type_t type, int32_t value) {
    if (type >= SENSOR_TYPE_COUNT || !is_screen_initialized) {
        return;
    }

    if (!charts[type].initialized) {
        if (value == LV_CHART_POINT_NONE) {
            return;
        }
        gui_sensmon_create_chart(type);
    }

    sensor_chart_t* sc = &charts[type];

    /* Update label */
    char new_value[32];
    switch (type) {
        case SENSOR_TEMPERATURE:
            snprintf(new_value, sizeof(new_value), "%ld.%ld °C", value / 100, value % 100);
            break;
        case SENSOR_HUMIDITY:
            snprintf(new_value, sizeof(new_value), "%ld.%ld %%", value / 100, value % 100);
            break;
        case SENSOR_PRESSURE:
            snprintf(new_value, sizeof(new_value), "%ld.%ld hPa", value / 100, value % 100);
            break;
        case SENSOR_TVOC:
            snprintf(new_value, sizeof(new_value), "%ld ppb", value);
            break;
        default:
            return;
    }

    if (value == LV_CHART_POINT_NONE) {
        snprintf(new_value, sizeof(new_value), "--");
    }
    
    lv_label_set_text(sc->curr_value, new_value);
}

void gui_sensmon_push_chart_value(sensor_data_type_t type, int32_t value) {
    if (type >= SENSOR_TYPE_COUNT || !is_screen_initialized) {
        return;
    }

    if (!charts[type].initialized) {
        if (value == LV_CHART_POINT_NONE) {
            return;
        }
        gui_sensmon_create_chart(type);
    }

    sensor_chart_t* sc = &charts[type];

    /* Store to buffer */
    if (sc->count < SENSOR_MONITOR_MAX_POINTS) {
        sc->values[sc->count++] = value;
    } else {
        for (int i = 1; i < SENSOR_MONITOR_MAX_POINTS; i++) {
            sc->values[i - 1] = sc->values[i];
        }
        sc->values[SENSOR_MONITOR_MAX_POINTS - 1] = value;
    }

    /* Determine Y range */
    int32_t min = sc->values[0];
    int32_t max = sc->values[0];
    for (int i = 1; i < sc->count; i++) {
        if (sc->values[i] == LV_CHART_POINT_NONE) {
            continue;
        }
        if (sc->values[i] < min) {
            min = sc->values[i];
        }
        if (sc->values[i] > max) {
            max = sc->values[i];
        }
    }
    lv_chart_set_range(sc->chart, LV_CHART_AXIS_PRIMARY_Y, (lv_coord_t)min, (lv_coord_t)max);

    /* Update chart series */
    lv_chart_set_next_value(sc->chart, sc->series, value);
    lv_chart_refresh(sc->chart);
}
