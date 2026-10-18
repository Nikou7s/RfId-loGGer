import serial
import time
import csv
import os
from datetime import datetime

SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200
CSV_FILE = 'attendance.csv'

# Add ALL your card UIDs and real names here
STUDENTS = {
    "7A 4B B7 4F": "Student One",
    "1A 2F B3 4F": "Student Two"
}

# Keep track of who has been logged today
daily_logs = {}

def load_existing_logs():
    """Read the CSV on startup to remember who already scanned today."""
    if not os.path.exists(CSV_FILE):
        return

    with open(CSV_FILE, mode='r') as file:
        reader = csv.reader(file)
        next(reader, None)  # Skip header
        for row in reader:
            if len(row) >= 3:
                date_str, _, uid = row[0], row[1], row[2]
                if date_str not in daily_logs:
                    daily_logs[date_str] = set()
                daily_logs[date_str].add(uid)

def log_attendance(uid):
    now = datetime.now()
    date_str = now.strftime("%Y-%m-%d")
    time_str = now.strftime("%H:%M:%S")
    name = STUDENTS.get(uid, "Unknown Card")

    if date_str not in daily_logs:
        daily_logs[date_str] = set()

    # If they already scanned today, show in console but skip writing to CSV
    if uid in daily_logs[date_str]:
        print(f"⚠️ Scan recognized: {name} ({uid}) is already marked present today.")
        return

    # First time today: write to CSV and save to memory
    daily_logs[date_str].add(uid)

    with open(CSV_FILE, mode='a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([date_str, time_str, uid, name])

    print(f"✅ Added to database: {name} ({uid}) at {time_str}")

def main():
    # Create file and header if it doesn't exist
    if not os.path.exists(CSV_FILE):
        with open(CSV_FILE, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(["Date", "Time", "UID", "Name"])

    load_existing_logs()

    print(f"Starting attendance logger on {SERIAL_PORT}...")
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print("Connected! Waiting for scans. Press Ctrl+C to stop.")

        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                if line.startswith("Card UID:"):
                    uid = line.replace("Card UID:", "").strip()
                    log_attendance(uid)
            time.sleep(0.1)

    except serial.SerialException as e:
        print(f"Error connecting: {e}")
    except KeyboardInterrupt:
        print("\nLogger stopped.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == '__main__':
    main()
