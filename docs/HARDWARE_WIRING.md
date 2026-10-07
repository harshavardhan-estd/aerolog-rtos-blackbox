# Hardware Wiring, Pinout & Schematic Specification

This guide documents the physical connections and Bill of Materials (BOM) for deploying **AeroLog-RTOS** on the **STM32 Black Pill (ARM Cortex-M4)** or **ESP32 DevKit**.

---

## 1. Pinout Mapping Table

| Peripheral | Signal | STM32 Black Pill Pin | ESP32 DevKit Pin | Function / Details |
| :--- | :--- | :--- | :--- | :--- |
| **W25Qxx SPI Flash** | `SCK` | **PA5** | **GPIO 18** | SPI Clock (up to 50 MHz) |
| **W25Qxx SPI Flash** | `MISO` | **PA6** | **GPIO 19** | SPI Master In / Slave Out |
| **W25Qxx SPI Flash** | `MOSI` | **PA7** | **GPIO 23** | SPI Master Out / Slave In |
| **W25Qxx SPI Flash** | `CS` | **PA4** | **GPIO 5** | SPI Chip Select (Active LOW) |
| **MPU6050 IMU** | `SCL` | **PB8** | **GPIO 22** | I2C Clock (400 kHz Fast-Mode) |
| **MPU6050 IMU** | `SDA` | **PB9** | **GPIO 21** | I2C Data (400 kHz Fast-Mode) |
| **Brownout Detect** | `IRQ` | **PA0** | **GPIO 4** | EXTI Interrupt (Falling edge below 2.85V) |
| **Green LED** | Anode | **PB1** | **GPIO 2** | System Health Heartbeat |
| **Blue LED** | Anode | **PB12** | **GPIO 12** | Flash Active (Write / Sector Erase) |
| **Red LED** | Anode | **PB13** | **GPIO 13** | Brownout Emergency Flush Latched |
| **UART1 Serial** | `TX` | **PA9** | **GPIO 1** | 115200 Baud Diagnostic Telemetry |
| **UART1 Serial** | `RX` | **PA10** | **GPIO 3** | 115200 Baud Diagnostic CLI Input |

---

## 2. Schematic Diagram (ASCII Architecture)

```
                 +-----------------------------------------------+
                 |       STM32 Black Pill (Cortex-M4 84MHz)      |
                 |                                               |
[PA5 (SPI1_SCK)] +-------------------+                           |
[PA6 (SPI1_MISO)]+-----------------+ |                           |
[PA7 (SPI1_MOSI)]+---------------+ | |                           |
[PA4 (SPI1_CS)]  +-------------+ | | |                           |
                 |             | | | |                           |
[PB8 (I2C1_SCL)] +-----------+ | | | |                           |
[PB9 (I2C1_SDA)] +---------+ | | | | |                           |
                 |         | | | | | |                           |
[PA0 (EXTI0)] ---+-[Brownout IRQ Pushbutton / Comparator]------- GND
                 |                                               |
[PB1 (LED_G)] ---+-[330R]--[>| (Green Health)]------------------- GND
[PB12(LED_B)] ---+-[330R]--[>| (Blue Flash)]--------------------- GND
[PB13(LED_R)] ---+-[330R]--[>| (Red Brownout)]------------------- GND
                 +---------|-|-|-|-|-|-+-------------------------+
                           | | | | | |
                           | | | | | +--------+
                           | | | | +--------+ |
                           | | | +--------+ | |
                           | | +--------+ | | |
                           | |          | | | |
                     +-----+---+      +-+-+-+-+---+
                     | MPU6050 |      |  W25Q64   |
                     | 6-Axis  |      | SPI Flash |
                     | IMU     |      | 8 MB NOR  |
                     +---------+      +-----------+
```

---

## 3. Bill of Materials (BOM)

| Item | Component | Quantity | Approximate Cost (USD) |
| :--- | :--- | :--- | :--- |
| 1 | STM32F401CCU6 Black Pill Board (or ESP32 DevKit) | 1 | $3.90 |
| 2 | W25Q64 / W25Q16 SPI NOR Flash Module | 1 | $1.20 |
| 3 | MPU6050 6-Axis Accelerometer & Gyroscope Module | 1 | $1.70 |
| 4 | Tactile Pushbutton (Brownout IRQ simulation) | 1 | $0.20 |
| 5 | 5mm Diffused LEDs (1x Green, 1x Blue, 1x Red) | 3 | $0.30 |
| 6 | 330 $\Omega$ 1/4W Current Limiting Resistors | 3 | $0.15 |
| 7 | Breadboard & Jumper Wire Assortment | 1 | $2.50 |
| **Total** | | | **~$9.95** |
