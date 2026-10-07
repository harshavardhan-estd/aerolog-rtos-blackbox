/**
 * @file aerolog_types.h
 * @brief Common types, binary log entry layout, and FreeRTOS event definitions.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#ifndef AEROLOG_TYPES_H
#define AEROLOG_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AEROLOG_RECORD_MAGIC          0xAE50U
#define AEROLOG_RECORD_FOOTER         0x0D0AU

/**
 * @brief System return codes.
 */
typedef enum {
    AEROLOG_OK                =  0,
    AEROLOG_ERR_GENERIC       = -1,
    AEROLOG_ERR_NULL_PTR      = -2,
    AEROLOG_ERR_FLASH_FULL    = -3,
    AEROLOG_ERR_CRC_CORRUPT   = -4,
    AEROLOG_ERR_TIMEOUT       = -5,
    AEROLOG_ERR_NOT_MOUNTED   = -6,
    AEROLOG_ERR_QUEUE_FULL    = -7
} aerolog_status_t;

/**
 * @brief FreeRTOS Event Group Synchronization Bitmasks.
 */
typedef enum {
    EVT_BIT_SYSTEM_READY        = (1UL << 0), ///< Base clock and peripherals initialized
    EVT_BIT_STORAGE_MOUNTED     = (1UL << 1), ///< SPI Flash ring buffer header scanned
    EVT_BIT_SENSORS_CALIBRATED  = (1UL << 2), ///< IMU gyro/accel biases established
    EVT_BIT_LOGGING_ACTIVE      = (1UL << 3), ///< Ring buffer recording live telemetry
    EVT_BIT_BROWNOUT_DETECTED   = (1UL << 4), ///< Supply rail dip - emergency flush triggered
    EVT_BIT_STORAGE_WRAPPED     = (1UL << 5)  ///< Circular buffer has wrapped around sector 0
} aerolog_event_bits_t;

/**
 * @brief Flight / Machinery Dynamic Operating Phases.
 */
typedef enum {
    PHASE_IDLE_PREFLIGHT  = 0,
    PHASE_NORMAL_CRUISE   = 1,
    PHASE_HIGH_DYNAMIC    = 2,
    PHASE_TURBULENCE_SHOCK= 3,
    PHASE_EMERGENCY_DECENT= 4,
    PHASE_CRASH_IMPACT    = 5
} flight_phase_t;

/**
 * @brief Compact 32-Byte Telemetry Sensor Payload.
 */
typedef struct __attribute__((packed)) {
    int16_t  accel_x_mg;       ///< X acceleration in milli-g (+/- 16g range)
    int16_t  accel_y_mg;       ///< Y acceleration in milli-g
    int16_t  accel_z_mg;       ///< Z acceleration in milli-g
    int16_t  gyro_x_dps;       ///< Roll angular rate (deg/sec)
    int16_t  gyro_y_dps;       ///< Pitch angular rate (deg/sec)
    int16_t  gyro_z_dps;       ///< Yaw angular rate (deg/sec)
    uint16_t bus_voltage_mv;   ///< System power rail in millivolts
    int16_t  mcu_temp_deci_c;  ///< Core temperature in deci-Celsius (e.g. 425 = 42.5 C)
    uint8_t  flight_phase;     ///< Enum: flight_phase_t
    uint8_t  alarm_flags;      ///< Bitmask of active system faults
    uint16_t padding;          ///< Alignment padding
} sensor_payload_t;

/**
 * @brief Fixed 48-Byte Binary Log Entry for SPI NOR Flash Storage with CRC32.
 */
typedef struct __attribute__((packed)) {
    uint16_t         magic_header;   ///< Magic sync word: 0xAE50
    uint32_t         record_seq;     ///< Monotonically increasing record ID
    uint32_t         timestamp_ms;   ///< System uptime in milliseconds
    sensor_payload_t payload;        ///< Sensor payload (20 bytes)
    uint32_t         crc32;          ///< CRC32 checksum over header & payload
    uint16_t         magic_footer;   ///< Magic footer: 0x0D0A (\r\n)
} log_record_t;

/**
 * @brief FreeRTOS Task Runtime Telemetry & Stack Watermark Statistics.
 */
typedef struct {
    uint32_t total_records_logged;
    uint32_t flash_write_head_addr;
    uint32_t flash_tail_read_addr;
    uint16_t queue_messages_waiting;
    uint16_t queue_high_water_mark;
    uint32_t sensors_stack_free_bytes;
    uint32_t storage_stack_free_bytes;
    uint32_t telemetry_stack_free_bytes;
    uint32_t cli_stack_free_bytes;
    float    estimated_cpu_load_pct;
    bool     brownout_latched;
} rtos_runtime_stats_t;

#ifdef __cplusplus
}
#endif

#endif /* AEROLOG_TYPES_H */
