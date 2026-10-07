/**
 * @file flash_ring_buffer.h
 * @brief Fail-Safe Circular NOR Flash Ring Buffer with Wear-Leveling and CRC32.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#ifndef FLASH_RING_BUFFER_H
#define FLASH_RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "aerolog_types.h"
#include "aerolog_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flash Ring Buffer Control Block.
 */
typedef struct {
    uint32_t write_head_addr;      ///< Current byte address for next write
    uint32_t records_logged;       ///< Total lifetime records appended
    uint32_t current_sector_index; ///< Current active 4 KB sector index (0 to 15)
    uint32_t wrap_around_count;    ///< Number of times buffer wrapped over sector 0
    bool     is_mounted;           ///< Initialization status flag
} flash_ring_t;

/**
 * @brief Computes standard IEEE 802.3 CRC32 checksum.
 *
 * @param[in] data   Input byte buffer.
 * @param[in] length Number of bytes.
 * @return uint32_t 32-bit CRC.
 */
uint32_t aerolog_compute_crc32(const uint8_t *data, size_t length);

/**
 * @brief Mounts flash ring buffer and recovers write head after reboot/power-loss.
 *
 * @param[out] ring Pointer to ring buffer instance.
 * @return aerolog_status_t AEROLOG_OK on successful mount.
 */
aerolog_status_t flash_ring_init(flash_ring_t *ring);

/**
 * @brief Appends a sensor payload into the circular black-box journal.
 *
 * Automatically handles sector boundaries, proactive sector erase, and CRC32.
 *
 * @param[in,out] ring       Ring buffer instance.
 * @param[in]     payload    Sensor measurement payload.
 * @param[out]    out_record Pointer to copy of committed record (optional).
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t flash_ring_append(flash_ring_t *ring,
                                   const sensor_payload_t *payload,
                                   log_record_t *out_record);

/**
 * @brief Reads and validates a log record at physical byte address.
 *
 * @param[in]  ring       Ring buffer instance.
 * @param[in]  address    Physical byte address in flash.
 * @param[out] out_record Destination record struct.
 * @return aerolog_status_t AEROLOG_OK if valid and CRC passes.
 */
aerolog_status_t flash_ring_read_at(const flash_ring_t *ring,
                                    uint32_t address,
                                    log_record_t *out_record);

/**
 * @brief Formats the entire ring buffer (erases all sectors to 0xFF).
 *
 * @param[in,out] ring Ring buffer instance.
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t flash_ring_format(flash_ring_t *ring);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_RING_BUFFER_H */
