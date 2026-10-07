/**
 * @file aerolog_cli.h
 * @brief Interactive FreeRTOS Diagnostic Serial Command-Line Interface.
 * @author Seerapu Harsha Vardhan (AeroLog-RTOS Project)
 * @license MIT
 */

#ifndef AEROLOG_CLI_H
#define AEROLOG_CLI_H

#include <stddef.h>
#include "aerolog_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Executes a single diagnostic command string and populates output buffer.
 *
 * @param[in]  cmd_line Input line string.
 * @param[out] output   Destination response buffer.
 * @param[in]  max_len  Maximum buffer size.
 * @return aerolog_status_t AEROLOG_OK on success.
 */
aerolog_status_t aerolog_cli_execute(const char *cmd_line, char *output, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* AEROLOG_CLI_H */
