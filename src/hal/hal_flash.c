/**
 * @file hal_flash.c
 * @brief High-fidelity SPI NOR Flash driver and physical emulation engine.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#include "hal/hal_flash.h"
#include <string.h>

/* Emulated 64 KB SPI NOR Flash Memory Space */
static uint8_t s_simulated_flash[FLASH_RING_TOTAL_CAPACITY];
static bool    s_flash_initialized = false;

aerolog_status_t hal_flash_init(void) {
    if (!s_flash_initialized) {
        memset(s_simulated_flash, 0xFF, sizeof(s_simulated_flash));
        s_flash_initialized = true;
    }
    return AEROLOG_OK;
}

aerolog_status_t hal_flash_read(uint32_t address, uint8_t *buffer, size_t length) {
    if (buffer == NULL) return AEROLOG_ERR_NULL_PTR;
    if (address + length > FLASH_RING_TOTAL_CAPACITY) return AEROLOG_ERR_FLASH_FULL;

    memcpy(buffer, &s_simulated_flash[address], length);
    return AEROLOG_OK;
}

aerolog_status_t hal_flash_write(uint32_t address, const uint8_t *buffer, size_t length) {
    if (buffer == NULL) return AEROLOG_ERR_NULL_PTR;
    if (address + length > FLASH_RING_TOTAL_CAPACITY) return AEROLOG_ERR_FLASH_FULL;

    /* Simulate true NOR Flash physics: bits can only transition 1 -> 0 */
    for (size_t i = 0; i < length; i++) {
        s_simulated_flash[address + i] &= buffer[i];
    }

    return AEROLOG_OK;
}

aerolog_status_t hal_flash_erase_sector(uint32_t sector_address) {
    uint32_t aligned_addr = (sector_address / FLASH_SECTOR_SIZE_BYTES) * FLASH_SECTOR_SIZE_BYTES;
    if (aligned_addr + FLASH_SECTOR_SIZE_BYTES > FLASH_RING_TOTAL_CAPACITY) {
        return AEROLOG_ERR_FLASH_FULL;
    }

    /* Reset sector to erased state (0xFF) */
    memset(&s_simulated_flash[aligned_addr], 0xFF, FLASH_SECTOR_SIZE_BYTES);
    return AEROLOG_OK;
}

aerolog_status_t hal_flash_erase_chip(void) {
    memset(s_simulated_flash, 0xFF, sizeof(s_simulated_flash));
    return AEROLOG_OK;
}
