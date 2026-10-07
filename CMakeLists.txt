cmake_minimum_required(VERSION 3.16)
project(AeroLogRTOS C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    include
    tests
)

# Common Core Sources
set(AEROLOG_CORE_SRCS
    src/core/rtos_tasks.c
    src/storage/flash_ring_buffer.c
    src/hal/hal_flash.c
    src/hal/hal_sensors.c
    src/cli/aerolog_cli.c
)

# Host Simulation Executable
add_executable(aerolog_sim
    src/main.c
    ${AEROLOG_CORE_SRCS}
)
target_link_libraries(aerolog_sim m)

# Unit Tests
enable_testing()

add_executable(test_flash_ring tests/test_flash_ring_buffer.c src/storage/flash_ring_buffer.c src/hal/hal_flash.c)
target_link_libraries(test_flash_ring m)
add_test(NAME TestFlashRing COMMAND test_flash_ring)

add_executable(test_rtos_primitives tests/test_rtos_primitives.c ${AEROLOG_CORE_SRCS})
target_link_libraries(test_rtos_primitives m)
add_test(NAME TestRTOSPrimitives COMMAND test_rtos_primitives)
