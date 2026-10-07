/**
 * @file hal_flash.h
 * @brief Hardware Abstraction Layer for SPI NOR Flash (W25Q series / Simulated).
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "aerolog_types.h"
#include "aerolog_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes SPI peripheral and flash chip select.
 *
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t hal_flash_init(void);

/**
 * @brief Reads data buffer from SPI flash memory.
 *
 * @param[in]  address Physical byte address in flash.
 * @param[out] buffer  Destination buffer.
 * @param[in]  length  Number of bytes to read.
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t hal_flash_read(uint32_t address, uint8_t *buffer, size_t length);

/**
 * @brief Programs a byte buffer to flash (must be in erased state).
 *
 * @param[in] address Physical byte address.
 * @param[in] buffer  Source data buffer.
 * @param[in] length  Number of bytes to program.
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t hal_flash_write(uint32_t address, const uint8_t *buffer, size_t length);

/**
 * @brief Erases a 4 KB hardware sector (sets all bits to 0xFF).
 *
 * @param[in] sector_address Any byte address within the 4 KB sector.
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t hal_flash_erase_sector(uint32_t sector_address);

/**
 * @brief Erases all sectors in the designated ring buffer region.
 *
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t hal_flash_erase_chip(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_FLASH_H */
