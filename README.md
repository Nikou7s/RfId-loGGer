# 🪪 RFID Attendance System

![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue?logo=espressif&logoColor=white)
![Docker](https://img.shields.io/badge/Container-Docker-2496ED?logo=docker&logoColor=white)
![Python](https://img.shields.io/badge/Python-3.11-3776AB?logo=python&logoColor=white)
![Flask](https://img.shields.io/badge/Web-Flask-black?logo=flask&logoColor=white)
![TailwindCSS](https://img.shields.io/badge/UI-TailwindCSS-06B6D4?logo=tailwindcss&logoColor=white)
![Cross-Platform](https://img.shields.io/badge/OS-Linux%20%7C%20Windows-lightgrey?logo=linux&logoColor=black)

A complete, production-grade attendance logging appliance powered by an **ESP32**, an **RC522 RFID reader**, an **SSD1306 0.96" OLED display**, and a containerized **Python/Flask** backend.

The system features real-time OLED feedback, live card deduplication against disk, persistent host storage via directory mounts, and a real-time **Teacher Dashboard** presenting a classroom grid of student tiles that illuminate green upon check-in, complete with manual absence/presence override capabilities.

---

## 📑 Table of Contents
1. [System Architecture](#-system-architecture)
2. [Hardware Components](#-hardware-components)
3. [Wiring & Pinout Reference](#-wiring--pinout-reference)
4. [Repository Structure](#-repository-structure)
5. [Linux Quickstart (Arch / Manjaro / Ubuntu)](#-linux-quickstart)
6. [Windows Installation & Setup Guide](#-windows-installation--setup-guide)
7. [Teacher Dashboard & Operation](#-teacher-dashboard--operation)
8. [Data Integrity & Deduplication](#-data-integrity--deduplication)
9. [Troubleshooting](#-troubleshooting)

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
                                                              ▼ (/dev/ttyUSB0 or COMx)
 ┌────────────────────────────────────────────────────────────────────────┐
 │ Host System (Linux Docker / Windows Environment)                       │
 │                                                                        │
 │  ┌─────────────────────────┐               ┌────────────────────────┐  │
 │  │  rfid-logger Service    ├──────────────►│     attendance.csv     │  │
 │  │  (Python Serial Reader) │ Directory     │  (Live Shared Records) │  │
 │  └─────────────────────────┘ Mount / Disk  └───────────┬────────────┘  │
 │                                                        │               │
 │  ┌─────────────────────────┐                           │               │
 │  │  rfid-dashboard Service │◄──────────────────────────┘               │
 │  │  (Flask + Tailwind UI)  │ (Port 5000: Real-time Green Grid)        │
 │  └─────────────────────────┘                                           │
 └────────────────────────────────────────────────────────────────────────┘
~~~

---

## 🧰 Hardware Components

| Component | Specification | Qty | Role |
|---|---|:---:|---|
| **ESP32 DevKit V1** | ESP-WROOM-32 (Dual-core 240 MHz, 30-pin / 36-pin) | 1 | Hardware controller, bus supervisor, UI renderer, serial transmitter |
| **MFRC522 RFID Module** | 13.56 MHz RFID Reader/Writer (SPI interface) | 1 | Scans MIFARE Classic 1K / ISO 14443A card UIDs |
| **SSD1306 OLED Display** | 0.96-inch, 128×64 resolution, Monochrome (I²C default `0x3C`) | 1 | Graphical idle animations, student verification, scan feedback |
| **RFID Cards / Keyfobs** | 13.56 MHz ISO 14443A passive tokens | 2+ | Student/Employee identification tokens |
| **Micro-USB Cable** | High-speed data + power sync cable | 1 | Microcontroller flashing and serial pipeline |
| **Jumper Wires** | Female-to-Female DuPont connectors | 12 | Breadboard-free pin interconnections |

---

## ⚡ Wiring & Pinout Reference

> ⚠️ **CRITICAL VOLTAGE WARNING:** Power both the RC522 and the SSD1306 OLED exclusively from **3.3V (`3V3`)**. Connecting 5V (`VIN`) to the RC522 will permanently damage the module. Always disconnect the USB cable before altering connections.

### 1. RC522 RFID Reader → ESP32 (SPI)

| RC522 Pin | ESP32 GPIO | Function | Description |
|:---:|:---:|---|---|
| **SDA / SS** | **GPIO 5** | SPI Chip Select (`SS`) | Hardware Slave Select line (NOT I²C SDA) |
| **SCK** | **GPIO 18** | SPI Clock (`SCK`) | Serial clock output from ESP32 |
| **MOSI** | **GPIO 23** | SPI Master Out Slave In | Data transmitted from ESP32 to RC522 |
| **MISO** | **GPIO 19** | SPI Master In Slave Out | Data received by ESP32 from RC522 |
| **RST** | **GPIO 27** | Hardware Reset | Module reset trigger line |
| **3.3V / VCC** | **3V3** | 3.3V Power Rail | **Connect to 3.3V only** |
| **GND** | **GND** | Ground | Common system ground |
| **IRQ** | *Unconnected* | Interrupt Request | Left disconnected (polling mode) |

### 2. SSD1306 OLED Display → ESP32 (I²C)

| OLED Pin | ESP32 GPIO | Function | Description |
|:---:|:---:|---|---|
| **SDA** | **GPIO 21** | I²C Serial Data | Hardware I²C data line |
| **SCL** | **GPIO 22** | I²C Serial Clock | Hardware I²C clock line |
| **VCC** | **3V3** | 3.3V Power Rail | Shared 3.3V supply |
| **GND** | **GND** | Ground | Common system ground |

---

## 📁 Repository Structure

~~~text
RfId-loGGer/
├── docker-compose.yml              # Multi-container orchestration (Logger + Web Dashboard)
├── README.md                       # Complete technical and cross-platform guide
├── firmware/
│   ├── HealthCheck/
│   │   └── HealthCheck.ino         # Diagnostics Step 1: Core MCU, RAM, & GPIO test
│   ├── CircuitTest/
│   │   └── CircuitTest.ino         # Diagnostics Step 2: I2C (0x3C) & SPI (0x92) validation
│   └── Mainsys/
│       └── Mainsys.ino             # Production firmware: OLED animations & serial output
├── logger/
│   ├── Dockerfile                  # Lightweight Python 3.11 serial listener image
│   ├── requirements.txt            # pyserial dependency
│   ├── logger.py                   # Disk-synchronized deduplication & CSV writer
│   └── attendance.csv              # Host-synced persistent attendance database
└── web/
    ├── Dockerfile                  # Dashboard container spec (Gunicorn WSGI)
    ├── requirements.txt            # Flask & Gunicorn dependencies
    ├── app.py                      # Dashboard routing, API endpoints, & roster management
    └── templates/
        └── index.html              # Responsive Tailwind CSS attendance grid UI
~~~

---

## 🐧 Linux Quickstart

### 1. Prerequisites & Toolchain Setup
On Arch / Manjaro:
~~~bash
sudo pacman -S --needed arduino-cli docker docker-compose
sudo usermod -aG uucp,docker \$USER
~~~
*(Log out and log back in to apply group permissions.)*

On Ubuntu / Debian:
~~~bash
sudo apt update && sudo apt install -y docker.io docker-compose
sudo usermod -aG dialout,docker \$USER
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
sudo mv bin/arduino-cli /usr/local/bin/
~~~

Install the ESP32 core and required hardware libraries:
~~~bash
arduino-cli core install esp32:esp32
arduino-cli lib install "Adafruit GFX Library" "Adafruit SSD1306" "MFRC522"
~~~

### 2. Flashing the Firmware
Verify the board port (`/dev/ttyUSB0`) and upload the main sketch:
~~~bash
cd firmware/Mainsys
arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
~~~

### 3. Launching Containers
Return to the project root and launch the logger and dashboard:
~~~bash
cd ../..
docker compose up --build -d
~~~
Access the teacher portal at **`http://localhost:5000`**.

---

## 🪟 Windows Installation & Setup Guide

Windows handles USB serial communication differently from Linux: serial devices are identified as **`COMx`** (e.g., `COM3`, `COM4`) instead of `/dev/ttyUSB0`, and Docker Desktop runs inside a WSL2 virtual machine.

### Step 1: Install Drivers & Identify COM Port
1. Connect your ESP32 to your PC using a micro-USB data cable.
2. If Windows does not automatically recognize the board, download and install the virtual COM port driver:
   * **Silicon Labs CP210x Driver:** [Download Official CP210x VCP Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   * **WCH CH340 Driver:** (If your ESP32 uses the CH340 chip) [Download CH340 Driver](http://www.wch-ic.com/downloads/CH341SER_EXE.html)
3. Open **Device Manager** (`Win + X` → **Device Manager**).
4. Expand **Ports (COM & LPT)** and note your port number (e.g., `Silicon Labs CP210x USB to UART Bridge (COM3)`).

---

### Step 2: Flash Firmware Using Arduino IDE or Arduino CLI

#### Option A: Using Arduino IDE (Recommended for Windows)
1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software).
2. Go to **File → Preferences**, and add this URL to **Additional Boards Manager URLs**:
   ~~~text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ~~~
3. Open **Tools → Board → Boards Manager**, search for `esp32` by Espressif Systems, and click **Install**.
4. Open **Tools → Manage Libraries**, and install:
   * `Adafruit GFX Library`
   * `Adafruit SSD1306`
   * `MFRC522`
5. Open `firmware/Mainsys/Mainsys.ino`.
6. Under **Tools**, configure:
   * **Board:** `ESP32 Dev Module`
   * **Port:** Select your board's COM port (e.g., `COM3`)
   * **Upload Speed:** `115200`
7. Click **Upload** (hold the physical **BOOT** button on the ESP32 if the console displays `Connecting......`).

#### Option B: Using Windows Terminal / PowerShell (Arduino CLI)
1. Install `arduino-cli` via `winget`:
   ~~~powershell
   winget install Arduino.ArduinoCLI
   ~~~
2. Initialize and install core packages:
   ~~~powershell
   arduino-cli core install esp32:esp32
   arduino-cli lib install "Adafruit GFX Library" "Adafruit SSD1306" "MFRC522"
   ~~~
3. Compile and flash (replace `COM3` with your actual port):
   ~~~powershell
   cd firmware\Mainsys
   arduino-cli compile --fqbn esp32:esp32:esp32 .
   arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
   ~~~

---

### Step 3: Running the System on Windows

Choose between running natively with Python on Windows or using Docker with USB forwarding.

#### Method 1: Native Windows Setup (Recommended & Simplest)
Because standard Windows Docker Desktop does not map hardware COM ports directly into Linux containers, running the serial logger via Python and the dashboard via Flask on the host is the most direct path:

1. Install [Python 3.11+](https://www.python.org/downloads/) (ensure **"Add Python to PATH"** is checked during setup).
2. Open PowerShell in the project root:
   ~~~powershell
   cd RfId-loGGer
   python -m venv venv
   .\venv\Scripts\Activate.ps1
   pip install -r logger/requirements.txt -r web/requirements.txt
   ~~~
3. In **Terminal 1** (Run the serial logger inside the `logger` folder on your COM port):
   ~~~powershell
   cd logger
   \$env:SERIAL_PORT="COM3"
   python logger.py
   ~~~
4. In **Terminal 2** (Run the web dashboard from the project root):
   ~~~powershell
   \$env:CSV_PATH="logger/attendance.csv"
   python web/app.py
   ~~~
5. Open **`http://localhost:5000`** in your browser.

#### Method 2: Docker Desktop + WSL2 with `usbipd-win` (Full Containerized Method)
If you prefer running the full multi-container Docker Compose stack on Windows:

1. Install [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop/) with the WSL2 backend enabled.
2. Install `usbipd-win` on Windows via PowerShell (Administrator):
   ~~~powershell
   winget install dorssel.usbipd-win
   ~~~
3. List USB devices and find your ESP32's `BUSID`:
   ~~~powershell
   usbipd list
   ~~~
4. Bind and attach the device to WSL2 (replace `1-4` with your ESP32's BUSID):
   ~~~powershell
   usbipd bind --busid 1-4
   usbipd attach --wsl --busid 1-4
   ~~~
5. Open your WSL2 distribution terminal (e.g., Ubuntu):
   ~~~bash
   ls -l /dev/ttyUSB*
   cd /path/to/RfId-loGGer
   docker compose up --build -d
   ~~~

---

## 🖥️ Teacher Dashboard & Operation

The dashboard is accessible from any web browser on the local network (`http://<host-ip>:5000`).

~~~text
+-----------------------------------------------------------------------------------+
|  [●] Attendance Grid View          ● 2 Present  |  ○ 0 Absent     [🗑️ Reset All]  |
|  Live syncing enabled • Target Date: [ 2026-10-18 ]                               |
+-----------------------------------------------------------------------------------+
|  ┌─────────────────────┐  ┌─────────────────────┐  ┌─────────────────────┐        |
|  │ ✓        PRESENT    │  │ ✓        PRESENT    │  │ ○        ABSENT     │        |
|  │                     │  │                     │  │                     │        |
|  │   STUDENT ONE       │  │   STUDENT TWO       │  │   STUDENT THREE     │        |
|  │   7A 4B B7 4F       │  │   1A 2F B3 4F       │  │   9C 11 E2 4A       │        |
|  │   🕒 11:48:55       │  │   🕒 11:48:53       │  │   Not Scanned       │        |
|  │ [ Mark Absent ]     │  │ [ Mark Absent ]     │  │ [ Mark Present ]    │        |
|  └─────────────────────┘  └─────────────────────┘  └─────────────────────┘        |
+-----------------------------------------------------------------------------------+
~~~

### Key UI Capabilities
* **Dynamic Student Tiles:** Every student on the roster displays as a square tile. Absent students remain in a neutral gray waiting state; upon check-in, their tile illuminates in **emerald green** showing their scan timestamp.
* **Mark Absent:** Clicking "Mark Absent" strips that student's record for the selected date from `attendance.csv` and returns the card to neutral gray.
* **Mark Present:** Allows manual check-in if a student forgets their physical RFID token.
* **Historical Date Navigation:** Use the date selector to inspect or audit attendance records from prior school days.
* **Seamless Background Polling:** The dashboard queries `/api/roster` via JavaScript every 3 seconds to update the screen without causing full-page reloads.

---

## 🔒 Data Integrity & Deduplication

* **Disk-Synchronized Deduplication:** `logger.py` checks `attendance.csv` on disk prior to writing new entries. If an already-scanned card is tapped again on the same day, the terminal warns `⚠️ Scan recognized: ... is already marked present today`, preventing duplicate records.
* **Unregistered Card Handling:** Unregistered tokens trigger an `X` icon on the OLED screen and are recorded with the name `Unknown Card` for auditing.
* **Persistent Shared Storage:** The `docker-compose.yml` mounts the `./logger` directory directly (`./logger:/app` and `./logger:/app/data`), avoiding Linux inode decoupling and keeping data synced across containers.

---

## 🛠️ Troubleshooting

| Problem | Cause | Solution |
|---|---|---|
| `Failed to connect to ESP32: No serial data received` | ESP32 bootloader auto-reset failure | Hold down the physical **BOOT** button on the ESP32 when `Connecting...` appears, then release when writing begins. |
| Windows: Port does not appear in Device Manager | Missing CP210x/CH340 driver or charge-only USB cable | Install the Silicon Labs CP210x or CH340 driver. Verify the USB cable supports data transfer. |
| Linux: `Permission denied: '/dev/ttyUSB0'` | Missing user serial group membership | Run `sudo usermod -aG uucp,docker $USER` (or `dialout` on Ubuntu) and re-login. |
| `Internal Server Error` when clicking Mark Absent | Read-only volume mount (`:ro`) | Ensure `docker-compose.yml` mounts `./logger:/app/data` with read/write permissions. |
| Card scans on OLED but webpage does not update | Docker inode decoupling or frozen memory cache | Ensure directory mounts are used in `docker-compose.yml` and `logger.py` reads disk state directly. |
| Windows Docker: `device not found: /dev/ttyUSB0` | Windows Docker cannot pass COM ports natively | Use **Method 1** (Native Python execution) or forward the port into WSL2 using `usbipd-win`. |
