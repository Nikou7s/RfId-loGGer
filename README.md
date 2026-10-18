# 🪪 RFID Attendance System

![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue?logo=espressif&logoColor=white)
![Docker](https://img.shields.io/badge/Container-Docker-2496ED?logo=docker&logoColor=white)
![Python](https://img.shields.io/badge/Python-3.11-3776AB?logo=python&logoColor=white)
![Linux](https://img.shields.io/badge/OS-Linux-FCC624?logo=linux&logoColor=black)

A complete, containerized RFID attendance logging system built with an **ESP32** microcontroller, an **RC522 RFID reader**, and an **SSD1306 0.96" OLED display**, integrated with a **Dockerized Python** backend. The system displays real-time student verification on the OLED screen, suppresses duplicate daily scans, and stores timestamped attendance records in a persistent host-mounted CSV file.

---

## 📑 Table of Contents
1. [System Architecture](#-system-architecture)
2. [Hardware Components](#-hardware-components)
3. [Wiring & Pinout](#-wiring--pinout)
4. [Repository Structure](#-repository-structure)
5. [Step-by-Step Setup Workflow](#-step-by-step-setup-workflow)
6. [Attendance Rules & Logic](#-attendance-rules--logic)
7. [Troubleshooting](#-troubleshooting)

---

## 🏛️ System Architecture

~~~text
  ┌──────────────────────┐        SPI (3.3V)        ┌──────────────────────┐
  │  RC522 RFID Reader   ├─────────────────────────►│                      │
  └──────────────────────┘                          │     ESP-WROOM-32     │
  ┌──────────────────────┐        I2C (0x3C)        │   Microcontroller    │
  │ SSD1306 0.96" OLED   ├─────────────────────────►│                      │
  └──────────────────────┘                          └──────────┬───────────┘
                                                               │ USB Serial (115200 baud)
                                                               ▼ (/dev/ttyUSB0)
  ┌────────────────────────────────────────────────────────────────────────┐
  │ Host Machine (Linux / Docker Engine)                                   │
  │                                                                        │
  │  ┌─────────────────────────┐               ┌────────────────────────┐  │
  │  │  rfid-logger Container  ├──────────────►│     attendance.csv     │  │
  │  │  (Python 3.11 + Serial) │  Host Volume  │  (Timestamped Records) │  │
  │  └─────────────────────────┘               └────────────────────────┘  │
  └────────────────────────────────────────────────────────────────────────┘
~~~

---

## 🧰 Hardware Components

| Component | Specification | Qty | Purpose |
|---|---|:---:|---|
| **ESP32 DevKit V1** | ESP-WROOM-32 (Dual-core 240 MHz) | 1 | Main controller, OLED UI rendering, and serial transmitter |
| **MFRC522 RFID Module** | 13.56 MHz Reader/Writer (SPI protocol) | 1 | Scans MIFARE Classic 1K / ISO 14443A card UIDs |
| **SSD1306 OLED Display** | 0.96", 128×64 resolution, Monochrome (I²C `0x3C`) | 1 | Displays real-time scan prompts, student names, and status icons |
| **RFID Cards / Key Fobs** | 13.56 MHz ISO 14443A passive tags | 2+ | Student identification tokens |
| **Micro-USB Cable** | Data + Power cable (CP210x / CH340) | 1 | Firmware flashing and serial communication |
| **Jumper Wires** | Female-to-Female DuPont wires | 12 | Direct pin-to-pin hardware connections |

---

## ⚡ Wiring & Pinout

> ⚠️ **VOLTAGE WARNING:** Connect both the RC522 reader and SSD1306 OLED to the **3.3V (`3V3`)** pin, **NOT 5V (`VIN`)**. Supplying 5V to the RC522 will permanently damage the chip. Always unplug the USB cable before wiring.

### 1. RC522 RFID Reader → ESP32 (SPI)

| RC522 Pin | ESP32 Pin | Function | Notes |
|:---:|:---:|---|---|
| **SDA (SS)** | **GPIO 5** | SPI Chip Select | Hardware Slave Select (NOT I²C SDA) |
| **SCK** | **GPIO 18** | SPI Clock | Serial Clock line |
| **MOSI** | **GPIO 23** | SPI MOSI | Master Out Slave In (ESP32 → RC522) |
| **MISO** | **GPIO 19** | SPI MISO | Master In Slave Out (RC522 → ESP32) |
| **RST** | **GPIO 27** | Reset | Hardware reset line |
| **3.3V** | **3V3** | Power (3.3V) | **3.3V rail only** |
| **GND** | **GND** | Ground | Common ground |
| **IRQ** | *Unconnected* | Interrupt | Not used |

### 2. SSD1306 OLED Display → ESP32 (I²C)

| OLED Pin | ESP32 Pin | Function | Notes |
|:---:|:---:|---|---|
| **SDA** | **GPIO 21** | I²C Data | Hardware I²C SDA |
| **SCL** | **GPIO 22** | I²C Clock | Hardware I²C SCL |
| **VCC** | **3V3** | Power (3.3V) | Shared 3.3V rail |
| **GND** | **GND** | Ground | Common ground |

---

## 📁 Repository Structure

~~~text
RfId-loGGer/
├── .gitignore                      # Ignores virtualenvs and build artifacts
├── docker-compose.yml              # Container orchestration and USB device mapping
├── README.md                       # Project documentation
├── firmware/
│   ├── HealthCheck/
│   │   └── HealthCheck.ino         # Step 1: MCU core, RAM, and GPIO pull-up check
│   ├── CircuitTest/
│   │   └── CircuitTest.ino         # Step 2: I2C (0x3C) and SPI (RC522) bus scanner
│   └── Mainsys/
│       └── Mainsys.ino             # Step 3: Main attendance firmware and OLED UI
└── logger/
    ├── Dockerfile                  # Python 3.11-slim container build file
    ├── requirements.txt            # Python dependencies (pyserial)
    ├── logger.py                   # Serial listener and daily deduplication script
    └── attendance.csv              # Persistent host-mounted attendance log
~~~

---

## 🚀 Step-by-Step Setup Workflow

Follow these steps in order to verify your microcontroller and wiring before running the full attendance system.

### Prerequisites

Install `arduino-cli`, Docker, and Docker Compose on your Linux host:

~~~bash
# Arch / Manjaro Linux
sudo pacman -S --needed arduino-cli docker docker-compose
sudo usermod -aG uucp,docker \$USER
~~~
*(Log out and log back in for group permissions to take effect.)*

Install the ESP32 core and required Arduino libraries:

~~~bash
arduino-cli core install esp32:esp32
arduino-cli lib install "Adafruit GFX Library" "Adafruit SSD1306" "MFRC522"
~~~

---

### Step 1: ESP32 Core Health Check

Tests the ESP32 processor, flash memory, heap RAM, and verifies that all project GPIO pins read `HIGH` on internal pull-ups before connecting external modules.

~~~bash
cd firmware/HealthCheck
arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
~~~
Verify that all pins report `OK`, then exit the serial monitor with `Ctrl+C`.

---

### Step 2: Circuit & Bus Diagnostic

Wire the RC522 and SSD1306 OLED according to the pinout tables above, then flash the diagnostic sketch to verify I²C and SPI communication.

~~~bash
cd ../CircuitTest
arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
~~~
Confirm that the monitor outputs:
* `SUCCESS: I2C device found at address 0x3C`
* `RC522 Firmware Version Read: 0x92` (or `0x91` / `0x12`) and `SUCCESS: RC522 is wired correctly and communicating.`

Exit the serial monitor with `Ctrl+C`.

---

### Step 3: Flash Main Attendance Firmware

1. Open `firmware/Mainsys/Mainsys.ino` and update the `students[]` array with your card UIDs and names:
   ~~~cpp
   Student students[] = {
     {"7A 4B B7 4F", "STUDENT ONE"},
     {"1A 2F B3 4F", "STUDENT TWO"}
   };
   ~~~
2. Compile and upload the firmware:
   ~~~bash
   cd ../Mainsys
   arduino-cli compile --fqbn esp32:esp32:esp32 .
   arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
   ~~~
3. The OLED display will boot into the `READY TO SCAN / Tap your card` idle screen.

---

### Step 4: Start the Dockerized Attendance Logger

Return to the root directory and start the Python logger container:

~~~bash
cd ../..
docker compose up --build
~~~

Tap an RFID card on the reader:
* **OLED Display:** Shows `CARD DETECTED`, the student's name (or `UNREGISTERED`), and a checkmark (`✓`) or reject (`X`) icon.
* **Docker Terminal:** Prints real-time confirmation (`✅ Added to database: ...`).
* **CSV Database:** Saves the entry to `logger/attendance.csv`.

---

## 🔒 Attendance Rules & Logic

* **Daily Duplicate Suppression:** Upon startup, `logger.py` loads all scans from the current date into memory. Scanning the same card again on the same day displays a warning in the terminal (`⚠️ Scan recognized: ... is already marked present today`) without writing duplicate rows to `attendance.csv`.
* **Unknown Card Handling:** Unregistered cards display an `X` icon and `UNKNOWN CARD` on the OLED screen, and are logged as `Unknown Card` in the backend for auditing.
* **Persistent Storage:** `attendance.csv` is bind-mounted from the host system into the container so attendance data persists across container restarts and rebuilds.

---

## 🛠️ Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| `Failed to connect to ESP32: No serial data received` | Bootloader auto-reset timing | Hold the **BOOT** button on the ESP32 while `Connecting...` is shown, then release once uploading starts. |
| `Permission denied: '/dev/ttyUSB0'` | Missing serial group permissions | Run `sudo usermod -aG uucp $USER` (Arch/Manjaro) or `dialout` (Debian/Ubuntu) and re-login. |
| `RC522: NOT RESPONDING` (`0x00` or `0xFF`) | Loose SPI wires or power issue | Confirm power is on `3V3` (not 5V) and re-check GPIO 5, 18, 19, 23, and 27. |
| `OLED: NOT DETECTED` | Loose I²C wiring | Verify `SDA` is on GPIO 21 and `SCL` is on GPIO 22, then run `CircuitTest`. |
| `Device or resource busy: '/dev/ttyUSB0'` | Serial port locked by another process | Stop the Docker container (`Ctrl+C`) or close `arduino-cli monitor` before uploading firmware. |
