/**
 * @file gui_sensmon.c
 * @brief Sensors Monitor screen GUI logic with data persistence and visibility management
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gui.h"
#include <stdio.h>
#include <string.h>

static bool ui_is_built = false;

/**
 * @brief Structure to hold UI elements for a single sensor display
 */
typedef struct {
    lv_obj_t* container;        /**< Container for one chart and its labels */
    lv_obj_t* label_container;  /**< Holds sensor name and current value */
    lv_obj_t* label_name;       /**< Sensor name (e.g., "Temperature") */
    lv_obj_t* label_curr_value; /**< Current sensor value string */
    lv_obj_t* chart;
    lv_chart_series_t* series;
} sensor_chart_ui_t;

/**
 * @brief Structure to hold persistent data for a single sensor type
 */
typedef struct {
    int32_t values[SENSOR_MONITOR_MAX_POINTS];
    uint8_t data_point_count;
    int32_t last_known_value;
    bool has_ever_received_data;
} sensor_persistent_data_t;

static lv_obj_t* sensmon_screen_main_container;
static sensor_chart_ui_t sensor_ui_elements[SENSOR_MONITOR_MAX_CHARTS];
static sensor_persistent_data_t persistent_sensor_data[SENSOR_MONITOR_MAX_CHARTS];

static uint8_t created_charts_count = 0;

void gui_sensmon_data_init(void) {
    lv_memset(persistent_sensor_data, 0, sizeof(persistent_sensor_data));
    for (int i = 0; i < SENSOR_MONITOR_MAX_CHARTS; i++) {
        persistent_sensor_data[i].last_known_value = LV_CHART_POINT_NONE;
        for (int j = 0; j < SENSOR_MONITOR_MAX_POINTS; j++) {
            persistent_sensor_data[i].values[j] = LV_CHART_POINT_NONE;
        }
    }
    lv_memset(sensor_ui_elements, 0, sizeof(sensor_ui_elements));
}

static void refresh_displayed_chart_layouts() {
    if (!ui_is_built || created_charts_count == 0 || !sensmon_screen_main_container) return;
    if (lv_obj_has_flag(sensmon_screen_main_container, LV_OBJ_FLAG_HIDDEN)) return;

    lv_coord_t container_content_height = lv_obj_get_content_height(sensmon_screen_main_container) - 30;
    lv_coord_t row_padding = lv_obj_get_style_pad_row(sensmon_screen_main_container, LV_PART_MAIN);

    lv_coord_t height_per_chart;
    if (created_charts_count == 1) {
        height_per_chart = container_content_height;
        if (height_per_chart < 120) {
            height_per_chart = 120; /**< Min height for a single chart */

        }
        if (height_per_chart > 250) {
            height_per_chart = 250; /**< Max reasonable height for a single chart */
        }
    } else {
        if (created_charts_count == 0) {
            height_per_chart = container_content_height;
        } else {
            height_per_chart = (container_content_height - (created_charts_count - 1) * row_padding) / created_charts_count;
        }
        if (height_per_chart < 60) {
            height_per_chart = 60; /** Minimum height for multiple charts */
        }
    }

    for (int i = 0; i < SENSOR_MONITOR_MAX_CHARTS; i++) {
        if (sensor_ui_elements[i].container != NULL) {
            lv_obj_set_height(sensor_ui_elements[i].container, height_per_chart);
            if (sensor_ui_elements[i].chart) {
                lv_chart_refresh(sensor_ui_elements[i].chart);
            }
        }
    }
}

static void ensure_chart_ui_created(sensor_data_type_t type) {
    if (!ui_is_built || type >= SENSOR_MONITOR_MAX_CHARTS || !sensmon_screen_main_container) {
        return;
    }
    if (sensor_ui_elements[type].container != NULL) {
        return;
    }

    sensor_chart_ui_t* sc_ui = &sensor_ui_elements[type];
    sc_ui->container = lv_obj_create(sensmon_screen_main_container);
    lv_obj_remove_style_all(sc_ui->container);
    lv_obj_set_width(sc_ui->container, lv_pct(100));
    lv_obj_set_layout(sc_ui->container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(sc_ui->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(sc_ui->container, 3, 0);
    lv_obj_set_style_pad_column(sc_ui->container, 5, 0);
    lv_obj_set_style_bg_color(sc_ui->container, lv_color_white(), 0);
    lv_obj_set_style_radius(sc_ui->container, 5, 0);
    lv_obj_set_style_shadow_opa(sc_ui->container, LV_OPA_30, 0);
    lv_obj_set_style_shadow_width(sc_ui->container, 5, 0);
    lv_obj_set_style_shadow_offset_y(sc_ui->container, 2, 0);

    sc_ui->label_container = lv_obj_create(sc_ui->container);
    lv_obj_remove_style_all(sc_ui->label_container);
    lv_obj_set_size(sc_ui->label_container, 100, lv_pct(100));
    lv_obj_set_layout(sc_ui->label_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(sc_ui->label_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sc_ui->label_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(sc_ui->label_container, 3, 0);

    sc_ui->label_name = lv_label_create(sc_ui->label_container);
    lv_label_set_long_mode(sc_ui->label_name, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(sc_ui->label_name, lv_pct(120));
    lv_obj_set_style_text_align(sc_ui->label_name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(sc_ui->label_name, &lv_font_montserrat_14, 0);
    switch (type) {
        case SENSOR_TEMPERATURE:
            lv_label_set_text(sc_ui->label_name, "Temperature");
            break;
        case SENSOR_HUMIDITY:
            lv_label_set_text(sc_ui->label_name, "Humidity");
            break;
        case SENSOR_PRESSURE:
            lv_label_set_text(sc_ui->label_name, "Pressure");
            break;
        case SENSOR_TVOC:
            lv_label_set_text(sc_ui->label_name, "TVOC");
            break;
        default:
            lv_label_set_text(sc_ui->label_name, "Unknown");
            break;
    }

    sc_ui->label_curr_value = lv_label_create(sc_ui->label_container);
    lv_label_set_long_mode(sc_ui->label_curr_value, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(sc_ui->label_curr_value, lv_pct(100));
    lv_obj_set_style_text_align(sc_ui->label_curr_value, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(sc_ui->label_curr_value, &lv_font_montserrat_16, 0);
    lv_obj_set_style_margin_top(sc_ui->label_curr_value, 5, 0);
    lv_label_set_text(sc_ui->label_curr_value, "--");

    sc_ui->chart = lv_chart_create(sc_ui->container);
    lv_obj_set_style_flex_grow(sc_ui->chart, 1, 0);
    lv_obj_set_height(sc_ui->chart, lv_pct(100));
    lv_chart_set_type(sc_ui->chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(sc_ui->chart, SENSOR_MONITOR_MAX_POINTS);
    lv_chart_set_update_mode(sc_ui->chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(sc_ui->chart, 3, SENSOR_MONITOR_MAX_POINTS);
    lv_obj_set_style_bg_color(sc_ui->chart, lv_color_hex(0xF0F0F0), 0);

    sc_ui->series = lv_chart_add_series(sc_ui->chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_all_value(sc_ui->chart, sc_ui->series, LV_CHART_POINT_NONE);
    lv_chart_refresh(sc_ui->chart);

    created_charts_count++;
}

static void refresh_all_chart_data_and_labels() {
    if (!ui_is_built || !sensmon_screen_main_container || lv_obj_has_flag(sensmon_screen_main_container, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    uint8_t new_charts_this_cycle = 0;
    for (sensor_data_type_t type = 0; type < SENSOR_MONITOR_MAX_CHARTS; type++) {
        if (persistent_sensor_data[type].has_ever_received_data && sensor_ui_elements[type].container == NULL) {
            ensure_chart_ui_created(type);
            new_charts_this_cycle++;
        }
    }

    for (sensor_data_type_t type = 0; type < SENSOR_MONITOR_MAX_CHARTS; type++) {
        if (sensor_ui_elements[type].container == NULL) {
            continue;
        }

        sensor_chart_ui_t* sc_ui = &sensor_ui_elements[type];
        sensor_persistent_data_t* p_data = &persistent_sensor_data[type];

        char value_str[32];
        if (p_data->last_known_value == LV_CHART_POINT_NONE) {
            lv_snprintf(value_str, sizeof(value_str), "--");
        } else {
            switch (type) {
                case SENSOR_TEMPERATURE:
                    lv_snprintf(value_str, sizeof(value_str), "%ld.%01ld°C", p_data->last_known_value / 100, (p_data->last_known_value % 100) / 10);
                    break;
                case SENSOR_HUMIDITY:
                    lv_snprintf(value_str, sizeof(value_str), "%ld.%01ld%%", p_data->last_known_value / 100, (p_data->last_known_value % 100) / 10);
                    break;
                case SENSOR_PRESSURE:
                    lv_snprintf(value_str, sizeof(value_str), "%ldhPa", p_data->last_known_value / 100);
                    break;
                case SENSOR_TVOC:
                    lv_snprintf(value_str, sizeof(value_str), "%ldppb", p_data->last_known_value);
                    break;
                default:
                    lv_snprintf(value_str, sizeof(value_str), "N/A");
                    break;
            }
        }
        lv_label_set_text(sc_ui->label_curr_value, value_str);

        lv_chart_set_all_value(sc_ui->chart, sc_ui->series, LV_CHART_POINT_NONE);
        for (int i = 0; i < p_data->data_point_count; i++) {
            lv_chart_set_next_value(sc_ui->chart, sc_ui->series, p_data->values[i]);
        }

        int32_t min_y = LV_COORD_MAX, max_y = LV_COORD_MIN;
        for (int i = 0; i < p_data->data_point_count; ++i) {
            if (p_data->values[i] != LV_CHART_POINT_NONE) {
                if (p_data->values[i] < min_y) min_y = p_data->values[i];
                if (p_data->values[i] > max_y) max_y = p_data->values[i];
            }
        }
        lv_chart_set_range(sc_ui->chart, LV_CHART_AXIS_PRIMARY_Y, (lv_coord_t)min_y, (lv_coord_t)max_y);
        lv_chart_refresh(sc_ui->chart);
    }

    refresh_displayed_chart_layouts();
}


void gui_sensmon_screen_create(lv_obj_t* parent_of_screens_base_layer) {
    if (ui_is_built) {
        return;
    }

    sensmon_screen_main_container = lv_obj_create(parent_of_screens_base_layer);
    lv_obj_remove_style_all(sensmon_screen_main_container);
    lv_obj_set_size(sensmon_screen_main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(sensmon_screen_main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(sensmon_screen_main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(sensmon_screen_main_container, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sensmon_screen_main_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(sensmon_screen_main_container, 5, 0);
    lv_obj_set_style_pad_row(sensmon_screen_main_container, 5, 0);

    lv_memset(sensor_ui_elements, 0, sizeof(sensor_ui_elements));
    created_charts_count = 0;

    ui_is_built = true;
    /* Hidden by default. gui_manager will make it visible when needed. */
    lv_obj_add_flag(sensmon_screen_main_container, LV_OBJ_FLAG_HIDDEN);
}

void gui_sensmon_screen_set_visibility(bool visible) {
    if (!ui_is_built || !sensmon_screen_main_container) {
        return;
    }

    if (visible) {
        lv_obj_clear_flag(sensmon_screen_main_container, LV_OBJ_FLAG_HIDDEN);
        refresh_all_chart_data_and_labels();
    } else {
        lv_obj_add_flag(sensmon_screen_main_container, LV_OBJ_FLAG_HIDDEN);
    }
}

bool gui_sensmon_screen_is_built(void) {
    return ui_is_built;
}

void gui_sensmon_update_current_value(sensor_data_type_t type, int32_t value) {
    if (type >= SENSOR_TYPE_COUNT) {
        return;
    }

    sensor_persistent_data_t* p_data = &persistent_sensor_data[type];
    bool was_first_data = !p_data->has_ever_received_data;

    p_data->last_known_value = value;
    if (value != LV_CHART_POINT_NONE) {
        p_data->has_ever_received_data = true;
    }

    if (ui_is_built && sensmon_screen_main_container && !lv_obj_has_flag(sensmon_screen_main_container, LV_OBJ_FLAG_HIDDEN)) {
        if (p_data->has_ever_received_data) {
            ensure_chart_ui_created(type);

            if (sensor_ui_elements[type].container) {
                char value_str[32];
                if (value == LV_CHART_POINT_NONE) {
                    lv_snprintf(value_str, sizeof(value_str), "--");
                } else {
                    switch (type) {
                        case SENSOR_TEMPERATURE:
                            lv_snprintf(value_str, sizeof(value_str), "%ld.%01ld°C", value / 100, (value % 100) / 10);
                            break;
                        case SENSOR_HUMIDITY:
                            lv_snprintf(value_str, sizeof(value_str), "%ld.%01ld%%", value / 100, (value % 100) / 10);
                            break;
                        case SENSOR_PRESSURE:
                            lv_snprintf(value_str, sizeof(value_str), "%ldhPa", value / 100);
                            break;
                        case SENSOR_TVOC:
                            lv_snprintf(value_str, sizeof(value_str), "%ldppb", value);
                            break;
                        default:
                            lv_snprintf(value_str, sizeof(value_str), "N/A");
                            break;
                    }
                }
                lv_label_set_text(sensor_ui_elements[type].label_curr_value, value_str);

                if (was_first_data && p_data->has_ever_received_data) {
                    refresh_displayed_chart_layouts();
                }
            }
        }
    }
}

void gui_sensmon_push_chart_value(sensor_data_type_t type, int32_t value) {
    if (type >= SENSOR_TYPE_COUNT) {
        return;
    }

    sensor_persistent_data_t* p_data = &persistent_sensor_data[type];
    bool was_first_data = !p_data->has_ever_received_data;

    p_data->last_known_value = value;
    if (value != LV_CHART_POINT_NONE) {
        p_data->has_ever_received_data = true;
    }

    if (p_data->data_point_count < SENSOR_MONITOR_MAX_POINTS) {
        p_data->values[p_data->data_point_count++] = value;
    } else {
        for (int i = 0; i < SENSOR_MONITOR_MAX_POINTS - 1; i++) {
            p_data->values[i] = p_data->values[i + 1];
        }
        p_data->values[SENSOR_MONITOR_MAX_POINTS - 1] = value;
    }

    if (ui_is_built && sensmon_screen_main_container && !lv_obj_has_flag(sensmon_screen_main_container, LV_OBJ_FLAG_HIDDEN)) {
        if (p_data->has_ever_received_data) {
            ensure_chart_ui_created(type);

            if (sensor_ui_elements[type].container) {
                sensor_chart_ui_t* sc_ui = &sensor_ui_elements[type];
                lv_chart_set_next_value(sc_ui->chart, sc_ui->series, value);

                int32_t min_y = LV_COORD_MAX, max_y = LV_COORD_MIN;
                for (int i = 0; i < p_data->data_point_count; i++) {
                    if (p_data->values[i] != LV_CHART_POINT_NONE) {
                        if (p_data->values[i] < min_y) min_y = p_data->values[i];
                        if (p_data->values[i] > max_y) max_y = p_data->values[i];
                    }
                }
                lv_chart_set_range(sc_ui->chart, LV_CHART_AXIS_PRIMARY_Y, (lv_coord_t)min_y, (lv_coord_t)max_y);
                if (was_first_data && p_data->has_ever_received_data) {
                    refresh_displayed_chart_layouts();
                }
            }
        }
    }
}
