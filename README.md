# 🪪 RFID Attendance System

A complete hardware and software stack for tracking attendance using an ESP32, RC522 RFID reader, I2C OLED display, and a Dockerized Python backend.

## 🛠️ Hardware Requirements
* ESP32 Development Board
* RC522 RFID Reader module
* SSD1306 0.96" OLED Display (I2C)
* Jumper wires

## 🚀 Installation & Setup Workflow

To ensure everything works perfectly, follow these steps in exact order. Do not skip to the main firmware until your hardware passes the first two tests.

### Step 1: Core Health Check
Verify your ESP32 CPU, memory, and pin multiplexers are functioning before wiring anything.
1. Flash `firmware/1_HealthCheck/1_HealthCheck.ino` to your board.
2. Open your serial monitor at 115200 baud.
3. Ensure all pins report `OK`.

### Step 2: Circuit Diagnostic
Wire up your OLED and RC522 reader. This script bypasses heavy graphics libraries to simply ping the I2C and SPI buses.
1. Flash `firmware/2_CircuitTest/2_CircuitTest.ino`.
2. Open your serial monitor at 115200 baud.
3. Verify both the OLED address (`0x3C`) and the RC522 firmware version are detected.

### Step 3: Main Attendance Firmware
Once your hardware passes Step 2, you are ready for the main system.
1. Open `firmware/3_MainSystem/3_MainSystem.ino`.
2. Update the `students[]` array with your actual RFID card UIDs and names.
3. Flash the code to the ESP32. Your OLED should now show the "READY TO SCAN" idle screen.

### Step 4: Start the Docker Logger
The ESP32 sends scan data over USB to a host machine. 
1. Navigate to the `logger/` directory.
2. Build the image: `docker build -t rfid-logger .`
3. Run the container and mount your physical serial port:
   ```bash
   docker run -it --device=/dev/ttyUSB0 -v \$(pwd)/attendance.csv:/app/attendance.csv rfid-logger
