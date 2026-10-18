import os
import csv
import serial
import re
from datetime import datetime

PORT = os.environ.get("SERIAL_PORT", "/dev/ttyUSB0")
BAUD = 115200
CSV_FILE = "attendance.csv"

# Pre-registered lookup map
STUDENTS = {
    "7A 4B B7 4F": "Student One",
    "1A 2F B3 4F": "Student Two"
}

def ensure_csv():
    if not os.path.exists(CSV_FILE) or os.path.getsize(CSV_FILE) == 0:
        with open(CSV_FILE, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Date", "Time", "UID", "Name"])

def is_already_marked_today(uid, today_str):
    ensure_csv()
    with open(CSV_FILE, mode="r", newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            row_uid = row.get("UID", "").strip().upper()
            row_date = row.get("Date", "").strip()
            if row_uid == uid.upper() and row_date == today_str:
                return True
    return False

def record_attendance(uid):
    ensure_csv()
    now = datetime.now()
    today_str = now.strftime("%Y-%m-%d")
    time_str = now.strftime("%H:%M:%S")
    name = STUDENTS.get(uid.upper(), "Unknown Card")

    # Check live disk state instead of frozen RAM cache
    if is_already_marked_today(uid, today_str):
        print(f"⚠️ Scan recognized: {name} ({uid}) is already marked present today.", flush=True)
        return

    with open(CSV_FILE, mode="a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow([today_str, time_str, uid.upper(), name])
        f.flush()

    print(f"✅ Added to database: {name} ({uid}) at {time_str}", flush=True)

def main():
    ensure_csv()
    print(f"Starting attendance logger on {PORT}...", flush=True)

    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print("Connected! Waiting for scans. Press Ctrl+C to stop.", flush=True)
    except Exception as e:
        print(f"❌ Serial connection error: {e}", flush=True)
        return

    uid_pattern = re.compile(r"Card UID:\s*([0-9A-Fa-f ]+)")

    while True:
        try:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            match = uid_pattern.search(line)
            if match:
                uid = match.group(1).strip().upper()
                record_attendance(uid)

        except KeyboardInterrupt:
            print("\nShutting down logger.", flush=True)
            break
        except Exception as e:
            print(f"Read error: {e}", flush=True)

if __name__ == "__main__":
    main()
