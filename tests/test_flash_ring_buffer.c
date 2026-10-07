/**
 * @file test_flash_ring_buffer.c
 * @brief Unit tests for NOR Flash circular ring buffer, wear-leveling, and CRC32.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#include "unity/unity.h"
#include "storage/flash_ring_buffer.h"
#include "hal/hal_flash.h"
#include <stdio.h>
#include <string.h>

static int s_tests_run = 0;
static int s_tests_passed = 0;

void test_crc32_accuracy(void) {
    s_tests_run++;
    const char *test_msg = "123456789";
    /* Standard IEEE 802.3 CRC32 for "123456789" is 0xCBF43926 */
    uint32_t crc = aerolog_compute_crc32((const uint8_t *)test_msg, 9);
    TEST_ASSERT_EQUAL_INT(0xCBF43926UL, crc);

    s_tests_passed++;
    printf("PASS: test_crc32_accuracy (0x%08X)\n", crc);
}

void test_flash_ring_append_and_read(void) {
    s_tests_run++;
    flash_ring_t ring;
    flash_ring_init(&ring);
    flash_ring_format(&ring);

    sensor_payload_t p = {
        .accel_x_mg = 120,
        .accel_y_mg = -45,
        .accel_z_mg = 980,
        .gyro_x_dps = 15,
        .gyro_y_dps = 0,
        .gyro_z_dps = -2,
        .bus_voltage_mv = 3300,
        .mcu_temp_deci_c = 420,
        .flight_phase = PHASE_NORMAL_CRUISE,
        .alarm_flags = 0,
        .padding = 0
    };

    log_record_t out_rec;
    aerolog_status_t status = flash_ring_append(&ring, &p, &out_rec);
    TEST_ASSERT_EQUAL_INT(AEROLOG_OK, status);
    TEST_ASSERT_EQUAL_INT(1, ring.records_logged);
    TEST_ASSERT_EQUAL_INT(sizeof(log_record_t), ring.write_head_addr);

    /* Read back record from address 0 */
    log_record_t read_rec;
    status = flash_ring_read_at(&ring, 0, &read_rec);
    TEST_ASSERT_EQUAL_INT(AEROLOG_OK, status);
    TEST_ASSERT_EQUAL_INT(1, read_rec.record_seq);
    TEST_ASSERT_EQUAL_INT(3300, read_rec.payload.bus_voltage_mv);
    TEST_ASSERT_EQUAL_INT(out_rec.crc32, read_rec.crc32);

    s_tests_passed++;
    printf("PASS: test_flash_ring_append_and_read (Record #1 Verified)\n");
}

void test_flash_ring_power_loss_recovery(void) {
    s_tests_run++;
    flash_ring_t ring1;
    flash_ring_init(&ring1);
    flash_ring_format(&ring1);

    sensor_payload_t p = { .bus_voltage_mv = 3280 };

    /* Append 5 records */
    for (int i = 0; i < 5; i++) {
        flash_ring_append(&ring1, &p, NULL);
    }
    uint32_t head_before = ring1.write_head_addr;

    /* Simulate sudden power loss & reboot: Initialize a fresh control block */
    flash_ring_t ring2;
    aerolog_status_t status = flash_ring_init(&ring2);
    TEST_ASSERT_EQUAL_INT(AEROLOG_OK, status);
    TEST_ASSERT_EQUAL_INT(5, ring2.records_logged);
    TEST_ASSERT_EQUAL_INT(head_before, ring2.write_head_addr);

    s_tests_passed++;
    printf("PASS: test_flash_ring_power_loss_recovery (Write Head Recovered at 0x%05X)\n", ring2.write_head_addr);
}

int main(void) {
    printf("--- Running Flash Ring Buffer & Integrity Tests ---\n");
    test_crc32_accuracy();
    test_flash_ring_append_and_read();
    test_flash_ring_power_loss_recovery();
    printf("Results: %d/%d passed.\n\n", s_tests_passed, s_tests_run);
    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
