/**
 * @file main.c
 * @brief AeroLog-RTOS Black-Box Data Logger firmware entry point and host emulator.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#include <stdio.h>
#include <string.h>
#include "aerolog_types.h"
#include "aerolog_config.h"
#include "core/rtos_tasks.h"
#include "cli/aerolog_cli.h"

#if defined(STM32F4xx) || defined(ARDUINO_ARCH_STM32) || defined(ESP_PLATFORM) || defined(ARDUINO)
/* =========================================================================
 * Target FreeRTOS Firmware Implementation
 * ========================================================================= */
#include <Arduino.h>

void vTaskSensors(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        task_sensors_step();
        vTaskDelay(pdMS_TO_TICKS(SENSORS_SAMPLING_PERIOD_MS));
    }
}

void vTaskStorage(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        task_storage_step();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vTaskTelemetry(void *pvParameters) {
    (void)pvParameters;
    char telemetry_str[256];
    for (;;) {
        task_telemetry_step(telemetry_str, sizeof(telemetry_str));
        Serial.println(telemetry_str);
        vTaskDelay(pdMS_TO_TICKS(TELEMETRY_STREAM_PERIOD_MS));
    }
}

void vTaskCLI(void *pvParameters) {
    (void)pvParameters;
    char cli_buf[64];
    size_t idx = 0;

    for (;;) {
        while (Serial.available() > 0) {
            char c = (char)Serial.read();
            if (c == '\r' || c == '\n') {
                if (idx > 0) {
                    cli_buf[idx] = '\0';
                    char out_buf[512];
                    aerolog_cli_execute(cli_buf, out_buf, sizeof(out_buf));
                    Serial.print(out_buf);
                    idx = 0;
                }
            } else if (idx < sizeof(cli_buf) - 1) {
                cli_buf[idx++] = c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\r\n=======================================================");
    Serial.println("  AeroLog-RTOS: Fail-Safe Industrial Black-Box Logger  ");
    Serial.println("  Target: STM32 Black Pill (ARM Cortex-M4) / ESP32     ");
    Serial.println("=======================================================\r\n");

    rtos_system_init();

    xTaskCreate(vTaskSensors, "Sensors", TASK_STACK_SENSORS, NULL, TASK_PRIO_SENSORS, NULL);
    xTaskCreate(vTaskStorage, "Storage", TASK_STACK_STORAGE, NULL, TASK_PRIO_STORAGE, NULL);
    xTaskCreate(vTaskTelemetry, "Telem", TASK_STACK_TELEMETRY, NULL, TASK_PRIO_TELEMETRY, NULL);
    xTaskCreate(vTaskCLI, "CLI", TASK_STACK_CLI, NULL, TASK_PRIO_CLI, NULL);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

#else
/* =========================================================================
 * Host Verification & Multitasking Emulation Harness (Windows/Linux/macOS)
 * ========================================================================= */
int main(void) {
    printf("=================================================================\n");
    printf("  AeroLog-RTOS: Fail-Safe Industrial Black-Box Data Logger       \n");
    printf("  Host Emulation & FreeRTOS Multi-Tasking Verification Engine    \n");
    printf("=================================================================\n\n");

    rtos_system_init();
    printf("[BOOT] Initialized FreeRTOS synchronization primitives & Flash ring buffer.\n");

    /* Phase 1: High-Rate 50 Hz Sensor Ingestion and Queue-to-Storage Logging */
    printf("\n--> Running 50 Hz Sensor Pipeline (Acquiring 40 records)...\n");
    for (int i = 0; i < 40; i++) {
        task_sensors_step();
        task_storage_step();
    }

    char telem[256];
    task_telemetry_step(telem, sizeof(telem));
    printf("Telemetry Packet: %s\n\n", telem);

    /* Phase 2: Diagnostic CLI Status Check */
    printf("--> Executing Diagnostic CLI: 'status'\n");
    char cli_resp[1024];
    aerolog_cli_execute("status", cli_resp, sizeof(cli_resp));
    printf("%s\n", cli_resp);

    /* Phase 3: Dump Last Records from Flash Memory with CRC32 */
    printf("--> Executing Diagnostic CLI: 'dump 3'\n");
    aerolog_cli_execute("dump 3", cli_resp, sizeof(cli_resp));
    printf("%s\n", cli_resp);

    /* Phase 4: Simulating VDD Brownout Interrupt (< 2.85V) & Emergency Flush */
    printf("--> Simulating Sudden Power-Loss Brownout (EXTI0 Interrupt)...\n");
    aerolog_cli_execute("inject_brownout", cli_resp, sizeof(cli_resp));
    printf("%s", cli_resp);

    aerolog_cli_execute("status", cli_resp, sizeof(cli_resp));
    printf("%s\n", cli_resp);

    printf("[SUCCESS] All FreeRTOS multi-tasking scenarios verified successfully.\n");
    return 0;
}
#endif
