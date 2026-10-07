/**
 * @file hal_sensors.c
 * @brief High-rate 50 Hz IMU and Power Supervisory sensor implementation.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#include "hal/hal_sensors.h"
#include "aerolog_config.h"
#include <string.h>
#include <math.h>

static bool           s_brownout_simulated = false;
static flight_phase_t s_phase = PHASE_NORMAL_CRUISE;
static uint32_t       s_sample_seq = 0;

aerolog_status_t hal_sensors_init(void) {
    s_brownout_simulated = false;
    s_phase = PHASE_NORMAL_CRUISE;
    s_sample_seq = 0;
    return AEROLOG_OK;
}

void hal_sensors_inject_brownout(bool enable) {
    s_brownout_simulated = enable;
}

bool hal_sensors_is_brownout_active(void) {
    return s_brownout_simulated;
}

void hal_sensors_set_phase(flight_phase_t phase) {
    s_phase = phase;
}

aerolog_status_t hal_sensors_sample(sensor_payload_t *out_payload) {
    if (out_payload == NULL) return AEROLOG_ERR_NULL_PTR;

    s_sample_seq++;
    float t = (float)s_sample_seq * 0.02f; /* 20ms step */

    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 1000; /* 1.0g gravity */
    int16_t gx = 0;
    int16_t gy = 0;
    int16_t gz = 0;
    uint16_t v_bus = s_brownout_simulated ? 2450U : 3310U; /* < 2.85V if brownout */

    switch (s_phase) {
        case PHASE_IDLE_PREFLIGHT:
            ax = 12;
            ay = -8;
            az = 998;
            break;

        case PHASE_NORMAL_CRUISE:
            ax = (int16_t)(80.0f * sinf(2.0f * 3.1415f * 0.5f * t));
            ay = (int16_t)(45.0f * cosf(2.0f * 3.1415f * 0.5f * t));
            az = (int16_t)(1000.0f + 50.0f * sinf(2.0f * 3.1415f * 2.0f * t));
            gx = (int16_t)(12.0f * sinf(t));
            gy = (int16_t)(8.0f * cosf(t));
            gz = 2;
            break;

        case PHASE_TURBULENCE_SHOCK:
            ax = (int16_t)(850.0f * sinf(2.0f * 3.1415f * 4.0f * t));
            ay = (int16_t)(620.0f * cosf(2.0f * 3.1415f * 3.5f * t));
            az = (int16_t)(1000.0f + 1450.0f * sinf(2.0f * 3.1415f * 5.0f * t));
            gx = 45;
            gy = 62;
            gz = 18;
            break;

        case PHASE_CRASH_IMPACT:
            /* Massive peak shock > 12g */
            ax = 4500;
            ay = -6200;
            az = 14800; /* 14.8g crash pulse */
            gx = 320;
            gy = -410;
            gz = 180;
            break;

        default:
            break;
    }

    out_payload->accel_x_mg = ax;
    out_payload->accel_y_mg = ay;
    out_payload->accel_z_mg = az;
    out_payload->gyro_x_dps = gx;
    out_payload->gyro_y_dps = gy;
    out_payload->gyro_z_dps = gz;
    out_payload->bus_voltage_mv = v_bus;
    out_payload->mcu_temp_deci_c = 415; /* 41.5 C */
    out_payload->flight_phase = (uint8_t)s_phase;
    out_payload->alarm_flags = s_brownout_simulated ? 0x01 : 0x00;
    out_payload->padding = 0;

    return AEROLOG_OK;
}
