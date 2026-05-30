# 💧 Smart Water Quality Monitoring System

> Real-time IoT water quality monitoring with automatic contamination control using ESP32, TDS sensor, flow meter, and Firebase.

![ESP32](https://img.shields.io/badge/ESP32-IoT-blue?style=flat-square&logo=espressif)
![Arduino](https://img.shields.io/badge/Arduino-C++-teal?style=flat-square&logo=arduino)
![Firebase](https://img.shields.io/badge/Firebase-RTDB-orange?style=flat-square&logo=firebase)
![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)

---

## 📌 Overview

This project monitors water quality in real-time using an ESP32 microcontroller. It reads **TDS (Total Dissolved Solids)** levels and **water flow rate**, classifies the water status, and automatically **closes a servo-controlled valve** if the water is detected as contaminated or poisonous. All data is logged to the Serial Monitor (CSV format) and can be pushed live to **Firebase Realtime Database**.

---

## ⚙️ Hardware Requirements

| Component | Description |
|---|---|
| ESP32 | Main microcontroller (Wi-Fi enabled) |
| TDS Sensor | Measures water purity in ppm (parts per million) |
| Water Flow Sensor | YF-S201 or similar, pulse-based |
| Servo Motor | Controls the water shut-off valve |
| Jumper Wires | For connections |
| Power Supply | 5V USB or battery |

---

## 🔌 Pin Configuration

| Pin | Function |
|---|---|
| GPIO 34 | TDS Sensor (Analog Input) |
| GPIO 27 | Flow Sensor (Interrupt Input) |
| GPIO 14 | Servo Motor (PWM Output) |

---

## 📊 Water Quality Classification

| TDS Value (ppm) | Status |
|---|---|
| < 50 | 🔴 **POISON** — Valve closes immediately |
| 50 – 300 | 🟢 **SAFE** — Normal flow |
| 301 – 500 | 🟡 **MODERATE** — Acceptable |
| > 500 | 🔴 **CONTAMINATED** — Valve closes immediately |

---

## 🚀 Features

- **Real-time TDS monitoring** every 5 seconds
- **Flow rate calculation** using pulse-counting interrupt (L/min)
- **Total volume tracking** (cumulative liters)
- **Automatic servo valve control** — closes on contamination, reopens when safe
- **Serial output** in both human-readable and CSV format for data logging
- **Firebase RTDB integration** (ready to enable — currently commented out)
- **Wi-Fi connected** via ESP32 built-in module

---

## 📁 Project Structure

```
smart-water-monitor/
├── water_monitor.ino     # Main Arduino sketch
├── README.md             # This file
└── circuit_diagram/      # (Add your wiring diagram here)
```

---

## 🔧 Setup & Installation

### 1. Install Arduino IDE
Download from [arduino.cc](https://www.arduino.cc/en/software)

### 2. Install ESP32 Board Support
In Arduino IDE → Preferences → Additional Board URLs:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Then: Tools → Board → Boards Manager → Search "esp32" → Install

### 3. Install Required Libraries
Go to **Sketch → Include Library → Manage Libraries** and install:
- `ESP32Servo` by Kevin Harrington
- `Firebase ESP Client` by Mobizt *(for Firebase — currently commented out)*

### 4. Configure Wi-Fi
In `water_monitor.ino`, update:
```cpp
#define WIFI_SSID     "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"
```

### 5. (Optional) Enable Firebase
Uncomment the Firebase sections and fill in:
```cpp
#define API_KEY      "your_firebase_api_key"
#define DATABASE_URL "your_project.firebasedatabase.app"
```

### 6. Upload
- Select Board: `ESP32 Dev Module`
- Select correct COM Port
- Click **Upload**

---

## 📟 Serial Monitor Output

**Human-readable format (every 5 seconds):**
```
------ WATER DATA ------
Flow Rate: 2.30 L/min
Total Liters: 0.19
TDS: 145.62 ppm
Status: SAFE
------------------------
```

**CSV format (for data logging):**
```
12500,2.30,0.19,145.62,SAFE
```
> CSV columns: `timestamp_ms, flowRate, totalLiters, tds, status`

---

## 🔥 Firebase Data Structure

When Firebase is enabled, data is written to:
```
/water/
  ├── flowRate     → float (L/min)
  ├── totalLiters  → float (L)
  ├── tds          → float (ppm)
  └── status       → string (SAFE / MODERATE / CONTAMINATED / POISON)
```

---

## 🧠 How the Valve Logic Works

```
Every 5 seconds:
  Read TDS → Classify water status
  
  If status == POISON or CONTAMINATED:
    → Close valve (servo to 0°)
    → Start 10-second timer
  
  After 10 seconds:
    → Re-read TDS
    → If safe (50–500 ppm): Open valve (servo to 90°)
    → If still bad: Reset timer and check again
```

---

## 👨‍💻 Author

**Gowdham Kumar C**
- 🎓 B.Sc Computer Science & Applications — SKASC, Coimbatore
- 💼 IoT Head — Google On Campus, SKASC
- 🔗 [LinkedIn](https://www.linkedin.com/in/gowdham-kumar)
- 🐙 [GitHub](https://github.com/Gowdham-Kumar-C-dev)
- 📧 gowdhamkumarc@gmail.com

---

## 📄 License

This project is licensed under the MIT License — feel free to use, modify, and share.
