/**
 * @file aerolog_config.h
 * @brief System configuration, task priorities, queue depths, and flash layout.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#ifndef AEROLOG_CONFIG_H
#define AEROLOG_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * FreeRTOS Task Configuration & Priority Hierarchy
 * ========================================================================= */
#define TASK_PRIO_SENSORS              4U       ///< High Priority: 50 Hz deterministic acquisition
#define TASK_PRIO_STORAGE              3U       ///< Medium-High: Flash write & wear-leveling
#define TASK_PRIO_TELEMETRY            2U       ///< Normal Priority: 10 Hz telemetry streaming
#define TASK_PRIO_CLI                  1U       ///< Low Priority: Background diagnostic shell

#define TASK_STACK_SENSORS             2048U    ///< Words
#define TASK_STACK_STORAGE             3072U    ///< Words
#define TASK_STACK_TELEMETRY           2048U    ///< Words
#define TASK_STACK_CLI                 2048U    ///< Words

#define AEROLOG_STORAGE_QUEUE_LEN      32U      ///< Queue depth for sensor log entries
#define AEROLOG_MUTEX_TIMEOUT_MS       100U     ///< Maximum block time for SPI Flash mutex

/* =========================================================================
 * Sampling & Telemetry Frequencies
 * ========================================================================= */
#define SENSORS_SAMPLING_RATE_HZ       50U      ///< 50 Hz (20ms interval)
#define SENSORS_SAMPLING_PERIOD_MS     (1000U / SENSORS_SAMPLING_RATE_HZ)
#define TELEMETRY_STREAM_PERIOD_MS     100U     ///< 10 Hz telemetry broadcast

/* =========================================================================
 * SPI NOR Flash Ring Buffer Layout (Wear-Leveling Parameters)
 * ========================================================================= */
#define FLASH_SECTOR_SIZE_BYTES        4096U    ///< 4 KB hardware erase sector (W25Q16/32/64)
#define FLASH_RING_TOTAL_SECTORS       16U      ///< 16 sectors = 64 KB circular black-box space
#define FLASH_RING_START_ADDR          0x00000000UL
#define FLASH_RING_TOTAL_CAPACITY      (FLASH_SECTOR_SIZE_BYTES * FLASH_RING_TOTAL_SECTORS)

/* Brownout Thresholds */
#define BROWNOUT_VOLTAGE_THRESHOLD_MV  2850U    ///< Trigger emergency flush if VDD < 2.85V

/* =========================================================================
 * Hardware Pin Mapping (STM32 Black Pill Cortex-M4)
 * ========================================================================= */
#define PIN_SPI1_SCK                   PA5      ///< SPI Flash Clock
#define PIN_SPI1_MISO                  PA6      ///< SPI Flash MISO
#define PIN_SPI1_MOSI                  PA7      ///< SPI Flash MOSI
#define PIN_FLASH_CS                   PA4      ///< SPI Flash Chip Select

#define PIN_I2C1_SCL                   PB8      ///< MPU6050 I2C Clock
#define PIN_I2C1_SDA                   PB9      ///< MPU6050 I2C Data

#define PIN_BROWNOUT_EXTI              PA0      ///< Brownout Detect Interrupt (EXTI0)

#define PIN_LED_SYS_HEALTH             PB1      ///< Green LED: FreeRTOS Scheduler running
#define PIN_LED_FLASH_ACTIVE           PB12     ///< Blue LED: Flash read/write active
#define PIN_LED_BROWNOUT_ALERT         PB13     ///< Red LED: Brownout emergency flush latched

#define PIN_UART1_TX                   PA9      ///< 115200 Baud Console
#define PIN_UART1_RX                   PA10

#ifdef __cplusplus
}
#endif

#endif /* AEROLOG_CONFIG_H */
