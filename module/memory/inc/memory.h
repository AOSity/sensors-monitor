/**
 * @file memory.h
 * @brief flash memory driver interface
 *
 * @author Andrii Horbul (andreyhorbggwp@gmail.com)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef union {
    struct {
        int32_t timestamp;
        uint32_t value;
    };
    uint8_t raw[8];
} memory_entry_t;

typedef struct {
    bool (*init)(void);
    void (*read)(uint8_t* buf, uint32_t addr, uint32_t len);
    void (*write)(const uint8_t* buf, uint32_t addr, uint32_t len);
    void (*erase_sector)(uint32_t addr);
    void (*erase_chip)(void);
    uint32_t(*get_id)(void);
    uint16_t sector_size;
} memory_driver_t;

extern memory_driver_t memory;

/**
 * @brief Fills memory driver structure
 * @note Shall be implemented in you flash memory IC driver source file
 */
void memory_init_driver(void);

/**
 * @brief Called when screen data transmit complete
 */
void memory_tx_complete_handler(void);

/**
 * @brief Called when screen data receive complete
 */
void memory_rx_complete_handler(void);