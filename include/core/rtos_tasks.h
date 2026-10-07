/**
 * @file rtos_tasks.h
 * @brief FreeRTOS task coordinators, queues, mutexes, and emergency notification handlers.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

#include <stdint.h>
#include <stdbool.h>
#include "aerolog_types.h"
#include "aerolog_config.h"
#include "storage/flash_ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes all FreeRTOS synchronization primitives:
 * Queues, Mutexes, EventGroups, and Software Timers.
 *
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t rtos_system_init(void);

/**
 * @brief Sensor Acquisition Task (50 Hz, High Priority).
 * Deterministically acquires IMU & voltage samples and pushes to storage queue.
 */
void task_sensors_step(void);

/**
 * @brief Flash Storage Writer Task (Medium-High Priority).
 * Dequeues sensor payloads and writes to flash ring buffer protected by mutex.
 */
void task_storage_step(void);

/**
 * @brief Telemetry Broadcast Task (10 Hz, Normal Priority).
 * Formats live system state and broadcasts over serial stream.
 */
void task_telemetry_step(char *out_telemetry, size_t max_len);

/**
 * @brief External Interrupt (EXTI) Brownout Simulation Handler.
 * Sends zero-latency Task Notification directly to storage task to flush pending cache.
 */
void isr_brownout_trigger(void);

/**
 * @brief Returns current system statistics, watermarks, and record counts.
 */
void rtos_get_runtime_stats(rtos_runtime_stats_t *stats);

/**
 * @brief Direct access to underlying flash ring buffer control block.
 */
flash_ring_t* rtos_get_flash_ring(void);

#ifdef __cplusplus
}
#endif

#endif /* RTOS_TASKS_H */
