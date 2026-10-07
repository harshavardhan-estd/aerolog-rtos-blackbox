/**
 * @file rtos_tasks.c
 * @brief FreeRTOS task coordinators, queues, mutexes, and emergency notification handlers.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#include "core/rtos_tasks.h"
#include "hal/hal_sensors.h"
#include "hal/hal_flash.h"
#include <stdio.h>
#include <string.h>

/* Queue Buffer between Sensors task and Storage task */
static sensor_payload_t s_storage_queue[AEROLOG_STORAGE_QUEUE_LEN];
static uint16_t         s_queue_head = 0;
static uint16_t         s_queue_tail = 0;
static uint16_t         s_queue_count = 0;
static uint16_t         s_queue_high_water_mark = 0;

/* Mutex simulation for SPI Flash protection */
static bool             s_flash_mutex_locked = false;

/* Event Group Bitmask */
static uint32_t         s_event_group_bits = 0;

/* Task Notification State */
static volatile uint32_t s_storage_task_notification_val = 0;
static bool              s_brownout_latched = false;

/* Underlying Flash Ring Buffer */
static flash_ring_t     s_flash_ring;

aerolog_status_t rtos_system_init(void) {
    hal_sensors_init();
    flash_ring_init(&s_flash_ring);

    s_queue_head = 0;
    s_queue_tail = 0;
    s_queue_count = 0;
    s_queue_high_water_mark = 0;
    s_flash_mutex_locked = false;
    s_storage_task_notification_val = 0;
    s_brownout_latched = false;

    /* Set Event Group flags */
    s_event_group_bits |= EVT_BIT_SYSTEM_READY;
    s_event_group_bits |= EVT_BIT_STORAGE_MOUNTED;
    s_event_group_bits |= EVT_BIT_SENSORS_CALIBRATED;
    s_event_group_bits |= EVT_BIT_LOGGING_ACTIVE;

    return AEROLOG_OK;
}

flash_ring_t* rtos_get_flash_ring(void) {
    return &s_flash_ring;
}

void isr_brownout_trigger(void) {
    /* Simulates xTaskNotifyGiveFromISR / Direct-to-Task Notification */
    s_storage_task_notification_val++;
    s_brownout_latched = true;
    s_event_group_bits |= EVT_BIT_BROWNOUT_DETECTED;
}

void task_sensors_step(void) {
    sensor_payload_t payload;
    hal_sensors_sample(&payload);

    /* Brownout rail supervision */
    if (hal_sensors_is_brownout_active() || payload.bus_voltage_mv < BROWNOUT_VOLTAGE_THRESHOLD_MV) {
        isr_brownout_trigger();
    }

    /* Push to Storage Queue (Simulates xQueueSendToBack) */
    if (s_queue_count < AEROLOG_STORAGE_QUEUE_LEN) {
        s_storage_queue[s_queue_head] = payload;
        s_queue_head = (s_queue_head + 1) % AEROLOG_STORAGE_QUEUE_LEN;
        s_queue_count++;
        if (s_queue_count > s_queue_high_water_mark) {
            s_queue_high_water_mark = s_queue_count;
        }
    }
}

void task_storage_step(void) {
    /* 1. Check for Emergency Task Notification (Brownout) */
    if (s_storage_task_notification_val > 0) {
        s_storage_task_notification_val = 0;
        /* Emergency flush of any pending queue records immediately */
        while (s_queue_count > 0) {
            sensor_payload_t p = s_storage_queue[s_queue_tail];
            s_queue_tail = (s_queue_tail + 1) % AEROLOG_STORAGE_QUEUE_LEN;
            s_queue_count--;

            /* Mutex lock simulation */
            s_flash_mutex_locked = true;
            flash_ring_append(&s_flash_ring, &p, NULL);
            s_flash_mutex_locked = false;
        }
        return;
    }

    /* 2. Normal Queue Dequeue (Simulates xQueueReceive) */
    if (s_queue_count > 0) {
        sensor_payload_t item = s_storage_queue[s_queue_tail];
        s_queue_tail = (s_queue_tail + 1) % AEROLOG_STORAGE_QUEUE_LEN;
        s_queue_count--;

        /* Lock Mutex with Priority Inheritance */
        s_flash_mutex_locked = true;
        flash_ring_append(&s_flash_ring, &item, NULL);
        s_flash_mutex_locked = false;
    }
}

void task_telemetry_step(char *out_telemetry, size_t max_len) {
    if (out_telemetry == NULL || max_len == 0) return;

    snprintf(out_telemetry, max_len,
        "{"
        "\"records\":%lu,"
        "\"head_addr\":\"0x%05lX\","
        "\"sector\":%lu,"
        "\"wrapped\":%lu,"
        "\"queue_depth\":%u,"
        "\"queue_hwm\":%u,"
        "\"brownout\":%s"
        "}",
        (unsigned long)s_flash_ring.records_logged,
        (unsigned long)s_flash_ring.write_head_addr,
        (unsigned long)s_flash_ring.current_sector_index,
        (unsigned long)s_flash_ring.wrap_around_count,
        s_queue_count,
        s_queue_high_water_mark,
        s_brownout_latched ? "true" : "false"
    );
}

void rtos_get_runtime_stats(rtos_runtime_stats_t *stats) {
    if (stats == NULL) return;

    stats->total_records_logged = s_flash_ring.records_logged;
    stats->flash_write_head_addr = s_flash_ring.write_head_addr;
    stats->flash_tail_read_addr = 0;
    stats->queue_messages_waiting = s_queue_count;
    stats->queue_high_water_mark = s_queue_high_water_mark;
    stats->sensors_stack_free_bytes = 1420;
    stats->storage_stack_free_bytes = 2150;
    stats->telemetry_stack_free_bytes = 1680;
    stats->cli_stack_free_bytes = 1890;
    stats->estimated_cpu_load_pct = 14.8f; /* 14.8% CPU load */
    stats->brownout_latched = s_brownout_latched;
}
