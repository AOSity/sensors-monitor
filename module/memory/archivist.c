/**
 * @file archivist.c
 * @brief Contains logics for processing (save, display) sensor readings
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#include "sensors.h"
#include "memory.h"
#include "gui.h"
#include "slog.h"
#include "rtc.h"
#include "datetime.h"
#include "cmsis_os2.h"
#include <stdbool.h>

#define SENSOR_READ_VALUE_PERIOD_S 30
#define CHART_PUSH_VALUE_PERIOD_S  300
#define MEMORY_SAVE_VALUE_PERIOD_S 600

#define MEMORY_SECTORS_PER_SENSOR 4

static volatile bool need_sensor_read = true;
static volatile bool need_chart_push = true;
static volatile bool need_memory_save = false;

static int32_t last_data[SENSOR_TYPE_COUNT] = {0};
static int32_t chart_push_data[SENSOR_TYPE_COUNT] = {0};
static int32_t memory_save_data[SENSOR_TYPE_COUNT] = {0};
static uint32_t memory_save_addr[SENSOR_TYPE_COUNT] = {0};
extern memory_driver_t memory;

static void reading_handler(sensor_data_type_t type, int32_t value) {
    last_data[type] = value;

    if (chart_push_data[type] == 0) {
        chart_push_data[type] = value;
    } else {
        chart_push_data[type] = value + (chart_push_data[type] - value) / 2;
    }

    memory_save_data[type] = chart_push_data[type];
}

static void sensor_read_periodic_cb(void* argument) {
    need_sensor_read = true;
}

static void chart_push_periodic_cb(void* argument) {
    need_chart_push = true;
}

static void memory_save_periodic_cb(void* argument) {
    need_memory_save = true;
}

static void memory_scan(void) {
    for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
        const uint32_t start_addr = type * memory.sector_size * MEMORY_SECTORS_PER_SENSOR;
        const uint32_t end_addr = start_addr + memory.sector_size * MEMORY_SECTORS_PER_SENSOR;
        for (uint32_t addr = start_addr; addr < end_addr; addr += sizeof(memory_entry_t)) {
            memory_entry_t entry = {0};
            memory.read(entry.raw, addr, sizeof(entry));
            if (entry.value == 0xFFFFFFFF) {
                memory_save_addr[type] = addr;
                SLOG_DEBUG("sensor type %u, addr to write 0x%06X", type, addr);
                break;
            }
        }
    }
}

static void memory_save(void) {
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);
    uint32_t timestamp = datetime_to_timestamp(2000 + date.Year, date.Month, date.Date, time.Hours, time.Minutes, 0);
    SLOG_DEBUG("sensor data save with timestamp %lu", timestamp);

    for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
        const uint32_t start_addr = type * memory.sector_size * MEMORY_SECTORS_PER_SENSOR;
        if (memory_save_addr[type] % memory.sector_size == 0) {
            memory.erase_sector(memory_save_addr[type]);
        }

        memory_entry_t entry;
        entry.timestamp = timestamp;
        entry.value = memory_save_data[type];
        memory.write(entry.raw, memory_save_addr[type], sizeof(entry));
        SLOG_DEBUG("sensor type %u value saved at 0x%06X", type, memory_save_addr[type]);
        memory_save_addr[type] += sizeof(memory_entry_t);
        memory_save_addr[type] = start_addr + (memory_save_addr[type] - start_addr) % (memory.sector_size * 2);
    }
}

static void memory_load_data_from_timestamp(sensor_data_type_t type, uint32_t timestamp, int32_t* values, uint16_t count) {
    for (uint16_t i = 0; i < count; i++) {
        values[i] = LV_CHART_POINT_NONE;
    }

    if (type >= SENSOR_TYPE_COUNT || count == 0) {
        SLOG_WARN("Invalid arguments to memory_load_data: type=%d, count=%u", type, count);
        return;
    }

    const uint32_t sensor_region_size_bytes = memory.sector_size * MEMORY_SECTORS_PER_SENSOR;

    if (sensor_region_size_bytes == 0 || sizeof(memory_entry_t) == 0) {
        SLOG_ERROR("Memory region size (0x%lX) or entry size (%u) is zero for type: %d",
            sensor_region_size_bytes, (unsigned int)sizeof(memory_entry_t), type);
        return;
    }

    const uint32_t total_slots_per_sensor = sensor_region_size_bytes / sizeof(memory_entry_t);
    if (total_slots_per_sensor == 0) {
        SLOG_WARN("No slots available in memory for type %d. Region: %luB, Entry: %uB",
            type, sensor_region_size_bytes, (unsigned int)sizeof(memory_entry_t));
        return;
    }

    const uint32_t sensor_start_addr = (uint32_t)type * sensor_region_size_bytes;
    const uint32_t sensor_end_addr = sensor_start_addr + sensor_region_size_bytes;

    uint32_t current_next_write_addr = memory_save_addr[type];

    uint32_t addr_of_newest_entry;
    bool buffer_is_effectively_empty = false;

    if (current_next_write_addr == sensor_start_addr) {
        addr_of_newest_entry = sensor_end_addr - sizeof(memory_entry_t);
        memory_entry_t entry_in_last_slot;
        memory.read(entry_in_last_slot.raw, addr_of_newest_entry, sizeof(entry_in_last_slot));
        if (entry_in_last_slot.timestamp == 0xFFFFFFFF || entry_in_last_slot.value == 0xFFFFFFFF) {
            buffer_is_effectively_empty = true;
        }
    } else {
        addr_of_newest_entry = current_next_write_addr - sizeof(memory_entry_t);
        memory_entry_t entry_before_next_write;
        memory.read(entry_before_next_write.raw, addr_of_newest_entry, sizeof(entry_before_next_write));
        if (entry_before_next_write.timestamp == 0xFFFFFFFF || entry_before_next_write.value == 0xFFFFFFFF) {
            buffer_is_effectively_empty = true;
        }
    }

    if (buffer_is_effectively_empty) {
        SLOG_DEBUG("Memory effectively empty for type %d. Target ts: %lu", type, timestamp);
        return;
    }

    memory_entry_t newest_entry_data;
    memory.read(newest_entry_data.raw, addr_of_newest_entry, sizeof(newest_entry_data));

    if (newest_entry_data.timestamp == 0xFFFFFFFF || newest_entry_data.value == 0xFFFFFFFF) {
        SLOG_WARN("Newest entry at 0x%06lX (type %d) is unwritten. Target ts: %lu", 
            addr_of_newest_entry, type, timestamp);
        return;
    }

    if (timestamp > newest_entry_data.timestamp) {
        SLOG_DEBUG("Timestamp %lu too new for type %d (newest entry ts: %lu).",
            timestamp, type, newest_entry_data.timestamp);
        return;
    }

    uint32_t search_iter_addr = addr_of_newest_entry;
    uint32_t data_read_start_addr = 0;

    for (uint32_t i = 0; i < total_slots_per_sensor; ++i) {
        memory_entry_t current_search_entry;
        memory.read(current_search_entry.raw, search_iter_addr, sizeof(current_search_entry));

        if (current_search_entry.timestamp == 0xFFFFFFFF || current_search_entry.value == 0xFFFFFFFF) {
            break;
        }

        if (current_search_entry.timestamp <= timestamp) {
            data_read_start_addr = search_iter_addr;
            break;
        }

        if (search_iter_addr == sensor_start_addr) {
            search_iter_addr = sensor_end_addr - sizeof(memory_entry_t);
        } else {
            search_iter_addr -= sizeof(memory_entry_t);
        }

        if (i > 0 && search_iter_addr == addr_of_newest_entry) {
            break;
        }
    }

    if (data_read_start_addr == 0) {
        SLOG_DEBUG("Timestamp %lu too old for type %d (no entry with ts <= target found). Newest ts was: %lu",
            timestamp, type, newest_entry_data.timestamp);
        return;
    }

    uint32_t current_read_addr = data_read_start_addr;
    SLOG_DEBUG("Type %d: Starting forward read from addr 0x%06lX for %u items. Target ts: %lu. Newest ts: %lu",
        type, current_read_addr, count, timestamp, newest_entry_data.timestamp);

    for (uint16_t i = 0; i < count; ++i) {
        if (current_read_addr == current_next_write_addr && current_next_write_addr != sensor_start_addr) {
            SLOG_DEBUG("Type %d: Reached next_write_addr 0x%06lX (non-wrapping) at item %u. Stopping read.",
                type, current_read_addr, i);
            break;
        }

        memory_entry_t entry_to_load;
        memory.read(entry_to_load.raw, current_read_addr, sizeof(entry_to_load));

        if (entry_to_load.timestamp == 0xFFFFFFFF || entry_to_load.value == 0xFFFFFFFF) {
            SLOG_DEBUG("Type %d: Hit unwritten slot at 0x%06lX (ts:0x%08lX) at item %u. Stopping read.",
                type, current_read_addr, entry_to_load.timestamp, i);
            break;
        }

        values[i] = entry_to_load.value;

        current_read_addr += sizeof(memory_entry_t);
        if (current_read_addr >= sensor_end_addr) {
            current_read_addr = sensor_start_addr;
        }
    }
    SLOG_DEBUG("Type %d: memory_load_data_from_timestamp completed. %u values processed.", type, count);
}

void archivist_task(void* argument) {
    osDelay(200);
    gui_init();
    gui_history_init_data_fetcher(memory_load_data_from_timestamp);

    memory_init_driver();
    memory.init();
    SLOG_DEBUG("memory id: 0x%06X", memory.get_id());

    while (!gui_is_datetime_configured()) {
        gui_process();
        osDelay(5);
    }

    for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
        memory_save_addr[type] = type * memory.sector_size * MEMORY_SECTORS_PER_SENSOR;
    }
    //memory_scan();

    osTimerId_t sensor_read_periodic = osTimerNew(sensor_read_periodic_cb, osTimerPeriodic, NULL, NULL);
    osTimerStart(sensor_read_periodic, SENSOR_READ_VALUE_PERIOD_S * 1000);
    osTimerId_t chart_push_periodic = osTimerNew(chart_push_periodic_cb, osTimerPeriodic, NULL, NULL);
    osTimerStart(chart_push_periodic, CHART_PUSH_VALUE_PERIOD_S * 1000);
    osTimerId_t memory_save_periodic = osTimerNew(memory_save_periodic_cb, osTimerPeriodic, NULL, NULL);
    osTimerStart(memory_save_periodic, MEMORY_SAVE_VALUE_PERIOD_S * 1000);

    for (;;) {
        if (need_sensor_read) {
            need_sensor_read = false;
            sensor_process_reading(reading_handler);
            for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
                if (last_data[type] != 0) {
                    gui_sensmon_update_current_value(type, last_data[type]);
                    last_data[type] = 0;
                } else {
                    gui_sensmon_update_current_value(type, LV_CHART_POINT_NONE);
                }
            }
        }
        if (need_chart_push) {
            need_chart_push = false;
            for (uint8_t type = 0; type < SENSOR_TYPE_COUNT; type++) {
                if (chart_push_data[type] != 0) {
                    gui_sensmon_push_chart_value(type, chart_push_data[type]);
                    chart_push_data[type] = 0;
                } else {
                    gui_sensmon_push_chart_value(type, LV_CHART_POINT_NONE);
                }
            }
        }
        if (need_memory_save) {
            need_memory_save = false;
            memory_save();
        }
        gui_process();
        osDelay(5);
    }
}
