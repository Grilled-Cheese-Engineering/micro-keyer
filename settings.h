#pragma once
#include <stdint.h>
#include "hardware/flash.h"
#include "hardware/sync.h"

#ifdef __cplusplus
extern "C" {
    #endif

    // C-compatible enum
    typedef enum {
        MODE_CDC_SERIAL = 0,
        MODE_HID,
        MODE_MIDI
    } UsbMode;

    // Use the last sector of a 2MB flash for settings
    #define FLASH_TARGET_OFFSET (2 * 1024 * 1024 - FLASH_SECTOR_SIZE)

    // C-compatible struct with memory alignment
    typedef struct __attribute__((aligned(FLASH_PAGE_SIZE))) {
        UsbMode current_mode;
        uint8_t padding[FLASH_PAGE_SIZE - sizeof(UsbMode)]; // Pad to page size
    } DeviceSettings;

    static inline UsbMode read_boot_mode(void) {
        const DeviceSettings* settings = (const DeviceSettings*)(XIP_BASE + FLASH_TARGET_OFFSET);
        // If flash is empty (0xFF), default to Serial
        if (settings->current_mode == (UsbMode)0xFF) return MODE_CDC_SERIAL;
        return settings->current_mode;
    }

    static inline void save_boot_mode(UsbMode new_mode) {
        // Use a valid enum value instead of 0 to satisfy the C++ compiler. 
        // The padding array will still be automatically zero-initialized.
        DeviceSettings settings = { MODE_CDC_SERIAL };
        settings.current_mode = new_mode;

        // Writing to flash requires disabling interrupts!
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
        flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t*)&settings, FLASH_PAGE_SIZE);
        restore_interrupts(ints);
    }

    #ifdef __cplusplus
}
#endif