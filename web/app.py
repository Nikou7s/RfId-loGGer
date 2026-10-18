import csv
import os
from datetime import datetime
from flask import Flask, render_template, request, jsonify, redirect, url_for

app = Flask(__name__)
CSV_PATH = os.environ.get("CSV_PATH", "/app/data/attendance.csv")

REGISTERED_STUDENTS = [
    {"name": "STUDENT ONE", "uid": "7A 4B B7 4F"},
    {"name": "STUDENT TWO", "uid": "1A 2F B3 4F"},
]

def ensure_csv_exists():
    if not os.path.exists(CSV_PATH) or os.path.getsize(CSV_PATH) == 0:
        os.makedirs(os.path.dirname(CSV_PATH), exist_ok=True)
        with open(CSV_PATH, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Date", "Time", "UID", "Name"])

def get_scans_for_date(target_date):
    ensure_csv_exists()
    scans = {}
    unknowns = []

    with open(CSV_PATH, mode="r", newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for raw_row in reader:
            row = {k.strip().lower(): v.strip() for k, v in raw_row.items() if k}
            row_date = row.get("date", "")
            if row_date == target_date:
                uid = row.get("uid", "").upper()
                name = row.get("name", "Unknown")
                time_str = row.get("time", "")

                if "unknown" in name.lower() or "unregistered" in name.lower():
                    unknowns.append({"uid": uid, "name": name, "time": time_str})
                else:
                    scans[uid] = time_str
                    scans[name.upper()] = time_str
    return scans, unknowns

def get_roster_payload(selected_date):
    scans, unknowns = get_scans_for_date(selected_date)
    student_boxes = []
    present_count = 0

    for s in REGISTERED_STUDENTS:
        is_present = (s["uid"].upper() in scans) or (s["name"].upper() in scans)
        checkin_time = scans.get(s["uid"].upper()) or scans.get(s["name"].upper(), None)

        if is_present:
            present_count += 1

        student_boxes.append({
            "name": s["name"],
            "uid": s["uid"],
            "is_present": is_present,
            "checkin_time": checkin_time
        })

    return {
        "student_boxes": student_boxes,
        "unknowns": unknowns,
        "present_count": present_count,
        "absent_count": len(REGISTERED_STUDENTS) - present_count,
        "total_registered": len(REGISTERED_STUDENTS),
        "now": datetime.now().strftime("%H:%M:%S")
    }

@app.route("/")
def dashboard():
    today = datetime.now().strftime("%Y-%m-%d")
    selected_date = request.args.get("date", today).strip()
    data = get_roster_payload(selected_date)
    return render_template(
        "index.html",
        selected_date=selected_date,
        today=today,
        **data
    )

@app.route("/api/roster")
def api_roster():
    today = datetime.now().strftime("%Y-%m-%d")
    selected_date = request.args.get("date", today).strip()
    return jsonify(get_roster_payload(selected_date))

@app.route("/mark-absent", methods=["POST"])
def mark_absent():
    ensure_csv_exists()
    target_uid = request.form.get("uid", "").strip().upper()
    target_date = request.form.get("date", "").strip()

    if target_uid and target_date:
        rows = []
        fieldnames = ["Date", "Time", "UID", "Name"]
        with open(CSV_PATH, mode="r", newline="", encoding="utf-8") as f:
            reader = csv.DictReader(f)
            if reader.fieldnames:
                fieldnames = reader.fieldnames
            for raw_row in reader:
                row_uid = raw_row.get("UID", "").strip().upper()
                row_date = raw_row.get("Date", "").strip()
                if not (row_uid == target_uid and row_date == target_date):
                    rows.append(raw_row)

        with open(CSV_PATH, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(rows)

    return redirect(url_for("dashboard", date=target_date))

@app.route("/mark-present", methods=["POST"])
def mark_present():
    ensure_csv_exists()
    target_uid = request.form.get("uid", "").strip().upper()
    target_name = request.form.get("name", "Student").strip()
    target_date = request.form.get("date", "").strip()
    now_time = datetime.now().strftime("%H:%M:%S")

    if target_uid and target_date:
        with open(CSV_PATH, mode="a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow([target_date, now_time, target_uid, target_name])

    return redirect(url_for("dashboard", date=target_date))

@app.route("/clear-data", methods=["POST"])
def clear_data():
    ensure_csv_exists()
    with open(CSV_PATH, mode="w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["Date", "Time", "UID", "Name"])
    return redirect(url_for("dashboard"))

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)
