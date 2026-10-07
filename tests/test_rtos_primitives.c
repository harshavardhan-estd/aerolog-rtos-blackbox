/**
 * @file test_rtos_primitives.c
 * @brief Unit tests for FreeRTOS queues, task notifications, and emergency brownout flush.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#include "unity/unity.h"
#include "core/rtos_tasks.h"
#include <stdio.h>

static int s_tests_run = 0;
static int s_tests_passed = 0;

void test_queue_piping_between_tasks(void) {
    s_tests_run++;
    rtos_system_init();

    /* Run sensor task step (pushes 1 item to queue) */
    task_sensors_step();

    rtos_runtime_stats_t stats;
    rtos_get_runtime_stats(&stats);
    TEST_ASSERT_EQUAL_INT(1, stats.queue_messages_waiting);

    /* Run storage task step (dequeues 1 item and writes to flash) */
    task_storage_step();

    rtos_get_runtime_stats(&stats);
    TEST_ASSERT_EQUAL_INT(0, stats.queue_messages_waiting);
    TEST_ASSERT_EQUAL_INT(1, stats.total_records_logged);

    s_tests_passed++;
    printf("PASS: test_queue_piping_between_tasks (Piped through Queue & Committed to Flash)\n");
}

void test_brownout_emergency_flush_notification(void) {
    s_tests_run++;
    rtos_system_init();

    /* Push 4 records into queue */
    for (int i = 0; i < 4; i++) {
        task_sensors_step();
    }

    rtos_runtime_stats_t stats;
    rtos_get_runtime_stats(&stats);
    TEST_ASSERT_EQUAL_INT(4, stats.queue_messages_waiting);

    /* Trigger Brownout Interrupt (simulates external power drop) */
    isr_brownout_trigger();

    /* Storage task executes emergency flush via Direct-to-Task Notification */
    task_storage_step();

    rtos_get_runtime_stats(&stats);
    /* All 4 records must have been immediately flushed to flash */
    TEST_ASSERT_EQUAL_INT(0, stats.queue_messages_waiting);
    TEST_ASSERT_TRUE(stats.brownout_latched);

    s_tests_passed++;
    printf("PASS: test_brownout_emergency_flush_notification (Flushed 4 records via Direct Task Notification)\n");
}

int main(void) {
    printf("--- Running FreeRTOS Multi-Tasking & IPC Tests ---\n");
    test_queue_piping_between_tasks();
    test_brownout_emergency_flush_notification();
    printf("Results: %d/%d passed.\n\n", s_tests_passed, s_tests_run);
    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
