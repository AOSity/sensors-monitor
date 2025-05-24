/**
 * @file gui_manager.c
 * @brief GUI screens manager and navigation
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "gui.h"
#include <stdio.h>

static lv_obj_t* screen_nav_bar;
static lv_obj_t* base_screen_layer;

static lv_obj_t* nav_labels[GUI_SCREEN_COUNT];
static gui_screen_id_t current_screen_id = GUI_SCREEN_SENSORS;

/* Screen lifecycle/visibility function pointers */
typedef void (*screen_build_ui_func_t)(lv_obj_t* parent_of_screens_base_layer);
typedef void (*screen_set_visibility_func_t)(bool visible);
typedef bool (*screen_is_built_func_t)(void);

/* For screens that are created/destroyed */
typedef void (*screen_create_func_t)(lv_obj_t* parent_of_screens_base_layer);
typedef void (*screen_destroy_func_t)(void);
typedef bool (*screen_is_active_func_t)(void);


typedef struct {
    const char* name;
    bool manage_visibility_only; /**< True if screen is built once and then hidden/shown */
    /* Functions for visibility management */
    screen_build_ui_func_t build_ui_fn;
    screen_set_visibility_func_t set_visibility_fn;
    screen_is_built_func_t is_built_fn;
    /* Functions for create/destroy management (used if manage_visibility_only is false) */
    screen_create_func_t create_fn;
    screen_destroy_func_t destroy_fn;
    screen_is_active_func_t is_active_fn;
} screen_descriptor_t;

static const screen_descriptor_t screen_descriptors[GUI_SCREEN_COUNT] = {
    [GUI_SCREEN_SENSORS] = {
        .name = "Sensors",
        .manage_visibility_only = true,
        .build_ui_fn = gui_sensmon_screen_create,
        .set_visibility_fn = gui_sensmon_screen_set_visibility,
        .is_built_fn = gui_sensmon_screen_is_built,
        .create_fn = NULL,
        .destroy_fn = NULL,
        .is_active_fn = NULL,
    },
    [GUI_SCREEN_HISTORY] = {
        .name = "History",
        .manage_visibility_only = false,
        .build_ui_fn = NULL,
        .set_visibility_fn = NULL,
        .is_built_fn = NULL,
        .create_fn = gui_history_screen_create,
        .destroy_fn = gui_history_screen_destroy,
        .is_active_fn = gui_history_screen_is_active
    },
};

static lv_style_t style_nav_label_default;
static lv_style_t style_nav_label_active;

static void nav_label_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* label = lv_event_get_target(e);
    gui_screen_id_t screen_id = (gui_screen_id_t)((uintptr_t)lv_obj_get_user_data(label));

    if (code == LV_EVENT_CLICKED) {
        if (screen_id != current_screen_id) {
            gui_switch_screen(screen_id);
        }
    }
}

static void update_nav_bar_highlight(void) {
    for (int i = 0; i < GUI_SCREEN_COUNT; i++) {
        if (nav_labels[i]) {
            if (i == current_screen_id) {
                lv_obj_remove_style(nav_labels[i], &style_nav_label_default, 0);
                lv_obj_add_style(nav_labels[i], &style_nav_label_active, 0);
            } else {
                lv_obj_remove_style(nav_labels[i], &style_nav_label_active, 0);
                lv_obj_add_style(nav_labels[i], &style_nav_label_default, 0);
            }
        }
    }
}

static void create_navigation_bar(lv_obj_t* parent) {
    lv_style_init(&style_nav_label_default);
    lv_style_set_text_font(&style_nav_label_default, &lv_font_montserrat_16);
    lv_style_set_pad_all(&style_nav_label_default, 5);
    lv_style_set_text_color(&style_nav_label_default, lv_color_darken(lv_palette_main(LV_PALETTE_GREY), 2));

    lv_style_init(&style_nav_label_active);
    lv_style_set_text_font(&style_nav_label_active, &lv_font_montserrat_16);
    lv_style_set_pad_all(&style_nav_label_active, 5);
    lv_style_set_text_color(&style_nav_label_active, lv_color_white());
    lv_style_set_bg_color(&style_nav_label_active, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_opa(&style_nav_label_active, LV_OPA_COVER);
    lv_style_set_radius(&style_nav_label_active, 5);

    screen_nav_bar = lv_obj_create(parent);
    lv_obj_remove_style_all(screen_nav_bar);
    lv_obj_set_size(screen_nav_bar, SCREEN_WIDTH, LV_SIZE_CONTENT);
    lv_obj_align(screen_nav_bar, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_layout(screen_nav_bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(screen_nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(screen_nav_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(screen_nav_bar, 3, 0);
    lv_obj_set_style_pad_column(screen_nav_bar, 10, 0);

    for (int i = 0; i < GUI_SCREEN_COUNT; i++) {
        nav_labels[i] = lv_label_create(screen_nav_bar);
        lv_label_set_text(nav_labels[i], screen_descriptors[i].name);
        lv_obj_add_flag(nav_labels[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(nav_labels[i], (void*)((uintptr_t)i));
        lv_obj_add_event_cb(nav_labels[i], nav_label_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_style(nav_labels[i], &style_nav_label_default, 0);
    }
}

void gui_manager_init(void) {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xE0E0E0), 0);

    create_navigation_bar(scr);

    /* Create a base layer for all screen contents */
    base_screen_layer = lv_obj_create(scr);
    lv_obj_remove_style_all(base_screen_layer);
    lv_obj_set_size(base_screen_layer, SCREEN_WIDTH, SCREEN_HEIGHT - lv_obj_get_height(screen_nav_bar));
    lv_obj_align_to(base_screen_layer, screen_nav_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_pad_all(base_screen_layer, 0, 0);

    /* Pre-build screens that are managed by visibility */
    for (gui_screen_id_t id = 0; id < GUI_SCREEN_COUNT; ++id) {
        const screen_descriptor_t* desc = &screen_descriptors[id];
        if (desc->manage_visibility_only) {
            if (desc->build_ui_fn) {
                desc->build_ui_fn(base_screen_layer);
            }
        }
    }

    /* Load the initial screen */
    gui_switch_screen(current_screen_id);
}

void gui_switch_screen(gui_screen_id_t screen_id_to_show) {
    const screen_descriptor_t* old_desc = &screen_descriptors[current_screen_id];
    const screen_descriptor_t* new_desc = &screen_descriptors[screen_id_to_show];

    if (old_desc->manage_visibility_only) {
        if (old_desc->set_visibility_fn) {
            old_desc->set_visibility_fn(false);
        }
    } else {
        if (old_desc->destroy_fn && old_desc->is_active_fn && old_desc->is_active_fn()) {
            old_desc->destroy_fn();
        }
    }

    if (new_desc->manage_visibility_only) {
        if (new_desc->is_built_fn && !new_desc->is_built_fn()) {
            if (new_desc->build_ui_fn) {
                new_desc->build_ui_fn(base_screen_layer);
            }
        }
        if (new_desc->set_visibility_fn) {
            new_desc->set_visibility_fn(true);
        }
    } else {
        if (new_desc->create_fn) {
            new_desc->create_fn(base_screen_layer);
        }
    }

    current_screen_id = screen_id_to_show;
    update_nav_bar_highlight();
}
