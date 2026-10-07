/**
 * @file flash_ring_buffer.c
 * @brief Fail-Safe Circular NOR Flash Ring Buffer implementation with CRC32.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#include "storage/flash_ring_buffer.h"
#include "hal/hal_flash.h"
#include <string.h>

/* IEEE 802.3 CRC32 Implementation */
uint32_t aerolog_compute_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFUL;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

aerolog_status_t flash_ring_format(flash_ring_t *ring) {
    if (ring == NULL) return AEROLOG_ERR_NULL_PTR;

    hal_flash_erase_chip();
    ring->write_head_addr = 0;
    ring->records_logged = 0;
    ring->current_sector_index = 0;
    ring->wrap_around_count = 0;
    ring->is_mounted = true;

    return AEROLOG_OK;
}

aerolog_status_t flash_ring_init(flash_ring_t *ring) {
    if (ring == NULL) return AEROLOG_ERR_NULL_PTR;

    hal_flash_init();
    memset(ring, 0, sizeof(flash_ring_t));

    const size_t rec_size = sizeof(log_record_t);
    uint32_t highest_seq = 0;
    uint32_t last_valid_addr = 0;
    bool found_any_record = false;
    uint32_t empty_slot_addr = 0;
    bool found_empty_slot = false;

    /* Scan Flash sequentially to recover write head and highest sequence after reboot */
    for (uint32_t addr = 0; addr + rec_size <= FLASH_RING_TOTAL_CAPACITY; addr += rec_size) {
        log_record_t rec;
        if (hal_flash_read(addr, (uint8_t *)&rec, rec_size) == AEROLOG_OK) {
            /* Check if erased slot (0xFF) */
            if (rec.magic_header == 0xFFFFU) {
                if (!found_empty_slot) {
                    empty_slot_addr = addr;
                    found_empty_slot = true;
                }
                continue;
            }

            /* Validate magic and CRC */
            if (rec.magic_header == AEROLOG_RECORD_MAGIC && rec.magic_footer == AEROLOG_RECORD_FOOTER) {
                size_t crc_payload_len = sizeof(log_record_t) - sizeof(uint32_t) - sizeof(uint16_t);
                uint32_t calc_crc = aerolog_compute_crc32((const uint8_t *)&rec, crc_payload_len);
                if (calc_crc == rec.crc32) {
                    if (rec.record_seq > highest_seq) {
                        highest_seq = rec.record_seq;
                        last_valid_addr = addr;
                        found_any_record = true;
                    }
                }
            }
        }
    }

    if (!found_any_record) {
        /* Blank chip or corrupted: initialize at address 0 and erase sector 0 */
        ring->write_head_addr = 0;
        ring->records_logged = 0;
        hal_flash_erase_sector(0);
    } else {
        ring->records_logged = highest_seq;
        /* Position write head immediately after the highest record */
        uint32_t next_addr = last_valid_addr + rec_size;
        if (next_addr + rec_size > FLASH_RING_TOTAL_CAPACITY) {
            next_addr = 0; /* Wrapped */
            ring->wrap_around_count++;
        }
        ring->write_head_addr = next_addr;
    }

    ring->current_sector_index = ring->write_head_addr / FLASH_SECTOR_SIZE_BYTES;
    ring->is_mounted = true;

    return AEROLOG_OK;
}

aerolog_status_t flash_ring_append(flash_ring_t *ring,
                                   const sensor_payload_t *payload,
                                   log_record_t *out_record) {
    if (ring == NULL || payload == NULL || !ring->is_mounted) {
        return AEROLOG_ERR_NOT_MOUNTED;
    }

    const size_t rec_size = sizeof(log_record_t);

    /* Check for wrap-around */
    if (ring->write_head_addr + rec_size > FLASH_RING_TOTAL_CAPACITY) {
        ring->write_head_addr = 0;
        ring->wrap_around_count++;
        /* Erase sector 0 upon wrap */
        hal_flash_erase_sector(0);
    }

    /* Proactive Sector Erase: If entering the beginning of any 4 KB sector */
    if ((ring->write_head_addr % FLASH_SECTOR_SIZE_BYTES) == 0) {
        hal_flash_erase_sector(ring->write_head_addr);
    }

    /* Assemble binary record */
    log_record_t record;
    memset(&record, 0, sizeof(record));
    record.magic_header = AEROLOG_RECORD_MAGIC;
    record.record_seq = ++ring->records_logged;
    record.timestamp_ms = ring->records_logged * 20; /* 50 Hz -> 20ms */
    record.payload = *payload;
    record.magic_footer = AEROLOG_RECORD_FOOTER;

    /* Compute CRC32 over magic + seq + timestamp + payload */
    size_t crc_len = sizeof(log_record_t) - sizeof(uint32_t) - sizeof(uint16_t);
    record.crc32 = aerolog_compute_crc32((const uint8_t *)&record, crc_len);

    /* Commit write to flash */
    aerolog_status_t status = hal_flash_write(ring->write_head_addr, (const uint8_t *)&record, rec_size);
    if (status != AEROLOG_OK) {
        return status;
    }

    if (out_record != NULL) {
        *out_record = record;
    }

    /* Advance write head */
    ring->write_head_addr += rec_size;
    ring->current_sector_index = ring->write_head_addr / FLASH_SECTOR_SIZE_BYTES;

    return AEROLOG_OK;
}

aerolog_status_t flash_ring_read_at(const flash_ring_t *ring,
                                    uint32_t address,
                                    log_record_t *out_record) {
    if (ring == NULL || out_record == NULL || !ring->is_mounted) {
        return AEROLOG_ERR_NOT_MOUNTED;
    }

    const size_t rec_size = sizeof(log_record_t);
    if (address + rec_size > FLASH_RING_TOTAL_CAPACITY) {
        return AEROLOG_ERR_FLASH_FULL;
    }

    aerolog_status_t status = hal_flash_read(address, (uint8_t *)out_record, rec_size);
    if (status != AEROLOG_OK) {
        return status;
    }

    /* Validate header and footer */
    if (out_record->magic_header != AEROLOG_RECORD_MAGIC ||
        out_record->magic_footer != AEROLOG_RECORD_FOOTER) {
        return AEROLOG_ERR_CRC_CORRUPT;
    }

    /* Validate CRC32 */
    size_t crc_len = sizeof(log_record_t) - sizeof(uint32_t) - sizeof(uint16_t);
    uint32_t calc_crc = aerolog_compute_crc32((const uint8_t *)out_record, crc_len);
    if (calc_crc != out_record->crc32) {
        return AEROLOG_ERR_CRC_CORRUPT;
    }

    return AEROLOG_OK;
}
