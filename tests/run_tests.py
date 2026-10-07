#!/usr/bin/env python3
"""
AeroLog-RTOS Automated Unit Test & Concurrency Verification Suite
"""

import os
import sys
import subprocess
import shutil

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD_DIR = os.path.join(ROOT_DIR, "build_test")

TESTS = [
    {
        "name": "SPI NOR Flash Circular Ring Buffer & CRC32 Integrity",
        "srcs": ["tests/test_flash_ring_buffer.c", "src/storage/flash_ring_buffer.c", "src/hal/hal_flash.c"],
        "bin": "test_flash_ring"
    },
    {
        "name": "FreeRTOS Task Queues, Notifications & Brownout Flush",
        "srcs": ["tests/test_rtos_primitives.c", "src/core/rtos_tasks.c", "src/storage/flash_ring_buffer.c", "src/hal/hal_flash.c", "src/hal/hal_sensors.c"],
        "bin": "test_rtos_primitives"
    }
]

def main():
    print("=" * 68)
    print("  AeroLog-RTOS FreeRTOS Multi-Tasking & Storage Test Suite")
    print("=" * 68)

    os.makedirs(BUILD_DIR, exist_ok=True)
    all_passed = True

    compiler = shutil.which("gcc") or shutil.which("clang")
    if not compiler:
        print("[ERROR] Neither GCC nor Clang compiler found on PATH.")
        sys.exit(1)

    print(f"[TOOLCHAIN] Compiler: {compiler}\n")

    for t in TESTS:
        print(f"--> Building & Running: {t['name']}")
        exe_path = os.path.join(BUILD_DIR, t['bin'] + (".exe" if os.name == "nt" else ""))
        full_srcs = [os.path.join(ROOT_DIR, s) for s in t['srcs']]

        cmd = [
            compiler,
            "-I" + os.path.join(ROOT_DIR, "include"),
            "-I" + os.path.join(ROOT_DIR, "tests"),
            "-Wall", "-Wextra",
            "-o", exe_path
        ] + full_srcs + ["-lm"]

        compile_res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if compile_res.returncode != 0:
            print(f"    [COMPILE FAIL] {t['name']}")
            print(compile_res.stderr)
            all_passed = False
            continue

        run_res = subprocess.run([exe_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if run_res.returncode == 0:
            print(f"    [PASS] {t['name']}")
            for line in run_res.stdout.strip().splitlines():
                if line.startswith("PASS:"):
                    print(f"      {line}")
        else:
            print(f"    [EXEC FAIL] {t['name']}")
            print(run_res.stdout)
            print(run_res.stderr)
            all_passed = False

    shutil.rmtree(BUILD_DIR, ignore_errors=True)

    print("-" * 68)
    if all_passed:
        print("ALL FREERTOS CONCURRENCY & STORAGE TEST SUITES PASSED (100%).")
        sys.exit(0)
    else:
        print("ONE OR MORE TEST SUITES FAILED.")
        sys.exit(1)

if __name__ == "__main__":
    main()
