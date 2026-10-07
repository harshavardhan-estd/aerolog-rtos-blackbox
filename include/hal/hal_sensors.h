/**
 * @file hal_sensors.h
 * @brief Hardware Abstraction Layer for 6-Axis IMU, Voltage ADC, and Brownout simulation.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#ifndef HAL_SENSORS_H
#define HAL_SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "aerolog_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes IMU, ADC, and power supervisory peripherals.
 */
aerolog_status_t hal_sensors_init(void);

/**
 * @brief Samples high-rate 50 Hz telemetry vector (accelerations, rates, voltages).
 *
 * @param[out] out_payload Destination measurement payload.
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t hal_sensors_sample(sensor_payload_t *out_payload);

/**
 * @brief Simulates power-supply brownout dip (< 2.85V) for emergency ISR testing.
 *
 * @param[in] enable If true, drops voltage rail to trigger brownout interrupt.
 */
void hal_sensors_inject_brownout(bool enable);

/**
 * @brief Checks if physical or simulated supply voltage is in brownout condition.
 */
bool hal_sensors_is_brownout_active(void);

/**
 * @brief Sets active flight phase for synthetic dynamic telemetry generator.
 */
void hal_sensors_set_phase(flight_phase_t phase);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SENSORS_H */
