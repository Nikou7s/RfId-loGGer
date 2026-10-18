cat << 'EOF' > README.txt
===============================================================================
                       RFID ATTENDANCE SYSTEM
===============================================================================

A production-grade, end-to-end RFID attendance logging solution built on an
ESP32 microcontroller, an RC522 RFID reader, an SSD1306 OLED display, and a
containerized Python/Docker backend with daily duplicate suppression and
host-volume CSV persistence.

-------------------------------------------------------------------------------
1. SYSTEM ARCHITECTURE
-------------------------------------------------------------------------------

  +--------------------+        SPI (3.3V)        +--------------------+
  |  RC522 RFID Reader |------------------------->|                    |
  +--------------------+                          |    ESP-WROOM-32    |
  +--------------------+        I2C (0x3C)        |  Microcontroller   |
  | SSD1306 0.96" OLED |------------------------->|                    |
  +--------------------+                          +---------+----------+
                                                            | USB Serial (115200 baud)
                                                            v (/dev/ttyUSB0)
  +---------------------------------------------------------------------------+
  | Host Machine (Linux / Docker Engine)                                      |
  |                                                                           |
  |   +--------------------------+               +------------------------+   |
  |   |  rfid-logger Container   |-------------->|     attendance.csv     |   |
  |   |  (Python 3.11 + Serial)  |  Host Volume  |  (Timestamped Records) |   |
  |   +--------------------------+               +------------------------+   |
  +---------------------------------------------------------------------------+

-------------------------------------------------------------------------------
2. HARDWARE COMPONENTS
-------------------------------------------------------------------------------

* ESP32 DevKit V1 (ESP-WROOM-32 / ESP32-D0WD-V3, Dual-core 240 MHz) - Qty: 1
* MFRC522 RFID Module (13.56 MHz Reader/Writer, SPI protocol)        - Qty: 1
* SSD1306 OLED Display (0.96", 128x64, Monochrome, I2C interface)    - Qty: 1
* RFID Cards / Key Fobs (13.56 MHz ISO 14443A MIFARE cards)          - Qty: 2+
* Micro-USB Cable (Data + Power, CP210x compatible)                  - Qty: 1
* Jumper Wires (DuPont female-to-female / male-to-female)            - Qty: 12

-------------------------------------------------------------------------------
3. WIRING & PINOUT REFERENCE
-------------------------------------------------------------------------------

WARNING: Connect both modules to 3.3V (3V3), NOT 5V. Supplying 5V to the
RC522 can destroy the chip. Disconnect USB power before rewiring.

A. RC522 RFID Reader -> ESP32 (SPI)
   - SDA / SS  -> GPIO 5  (Hardware Chip Select; NOT I2C SDA)
   - SCK       -> GPIO 18 (SPI Clock)
   - MOSI      -> GPIO 23 (Master Out Slave In)
   - MISO      -> GPIO 19 (Master In Slave Out)
   - RST       -> GPIO 27 (Reset)
   - 3.3V/VCC  -> 3V3     (Power - 3.3V rail only)
   - GND       -> GND     (Ground)
   - IRQ       -> UNCONNECTED

B. SSD1306 OLED Display -> ESP32 (I2C)
   - SDA       -> GPIO 21 (I2C Data)
   - SCL       -> GPIO 22 (I2C Clock)
   - VCC       -> 3V3     (Shared 3.3V rail)
   - GND       -> GND     (Ground)

-------------------------------------------------------------------------------
4. REPOSITORY STRUCTURE
-------------------------------------------------------------------------------

RfId-loGGer/
|-- .gitignore               # Ignores build artifacts and virtualenvs
|-- docker-compose.yml       # Docker deployment and device mapping
|-- README.md                # Markdown technical guide
|-- README.txt               # Plain text technical documentation
|-- firmware/
|   |-- 1_HealthCheck/
|   |   `-- 1_HealthCheck.ino # Microcontroller and GPIO pull-up self-test
|   |-- 2_CircuitTest/
|   |   `-- 2_CircuitTest.ino # I2C (0x3C) and SPI (0x92) bus scanner
|   `-- 3_MainSystem/
|       `-- 3_MainSystem.ino  # Full attendance UI and serial emitter
`-- logger/
    |-- Dockerfile           # Python 3.11-slim container image
    |-- requirements.txt     # Python dependencies (pyserial)
    |-- logger.py            # Serial reader and daily deduplicator
    `-- attendance.csv       # Persistent attendance records

-------------------------------------------------------------------------------
5. SETUP & EXECUTION WORKFLOW
-------------------------------------------------------------------------------

PREREQUISITES:
Run on your Linux host:
  sudo pacman -S arduino-cli docker docker-compose
  sudo usermod -aG uucp,docker \$USER
  arduino-cli core install esp32:esp32
  arduino-cli lib install "Adafruit GFX Library" "Adafruit SSD1306" "MFRC522"

STEP 1: CORE HEALTH CHECK
1. cd firmware/1_HealthCheck
2. arduino-cli compile --fqbn esp32:esp32:esp32 .
3. arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
4. arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
5. Verify CPU, RAM, and pins report "OK". Exit monitor with Ctrl+C.

STEP 2: CIRCUIT TEST
1. cd ../2_CircuitTest
2. arduino-cli compile --fqbn esp32:esp32:esp32 .
3. arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
4. arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
5. Confirm OLED at 0x3C and RC522 firmware version 0x92/0x91/0x12.
   Exit monitor with Ctrl+C.

STEP 3: MAIN SYSTEM FIRMWARE
1. cd ../3_MainSystem
2. Open 3_MainSystem.ino and update student names and card UIDs.
3. arduino-cli compile --fqbn esp32:esp32:esp32 .
4. arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --upload-property upload.speed=115200 .
5. The OLED screen displays "READY TO SCAN".

STEP 4: DOCKER ATTENDANCE LOGGER
1. cd ../..
2. docker compose up --build
3. Tap an RFID card against the reader.
   - OLED displays student name and status.
   - Terminal logs confirmation.
   - Entry is stored in logger/attendance.csv.

-------------------------------------------------------------------------------
6. BUSINESS RULES & DEDUPLICATION
-------------------------------------------------------------------------------

* Daily Deduplication: Existing daily scans are indexed in memory on startup.
  Subsequent scans by the same card on the same date will not write duplicate
  lines to attendance.csv.
* Unregistered Cards: Tokens not listed in the database trigger an "X" icon on
  the OLED, display "UNKNOWN CARD / STATUS: REJECTED", and are recorded as
  "Unknown Card" in the logger.
* Persistence: The CSV file is mounted from the host filesystem, preventing
  data loss during container updates or restarts.

-------------------------------------------------------------------------------
7. TROUBLESHOOTING
-------------------------------------------------------------------------------

* ESP32 upload stalls at "Connecting...":
  Hold the BOOT button on the board until flashing begins, then release.
* Permission denied on /dev/ttyUSB0:
  Ensure your user is in the uucp group: `sudo usermod -aG uucp $USER`,
  then log out and log back in.
* RC522 not responding (0x00 or 0xFF):
  Check 3.3V power and verify SPI pin assignments (5, 18, 19, 23, 27).
* SSD1306 allocation failed:
  Verify SDA is on GPIO 21 and SCL is on GPIO 22. Run CircuitTest.
* Port busy during upload:
  Stop the Docker container or serial monitor before flashing.
===============================================================================
EOF
