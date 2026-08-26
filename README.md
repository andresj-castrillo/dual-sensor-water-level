# dual-sensor-water-level

Real-time water level monitor on ESP32 using two independent, redundant measurement methods: a **load cell (HX711)** that measures by weight, and an **ultrasonic sensor (HC-SR04)** that measures by distance. The ESP32 serves its own web dashboard from LittleFS, with live readings, remote calibration, and CSV history export — no external backend required.

## Features

- **Two independent sensors**: weight (HX711) and distance (HC-SR04), each with its own endpoint, calibration flow, and persistence.
- **Median filtering** over 7 samples on the ultrasonic sensor to discard erratic readings.
- **Mandatory serial calibration** on boot (tare + known weight for the load cell; reference distance + tank reference height for the ultrasonic sensor), stored in EEPROM so it survives reboots.
- **Embedded web server** (`WebServer.h`) with JSON endpoints for both sensors plus configuration/tare endpoints.
- **Dashboard served from LittleFS**: tank-style level visualization, low/high level alerts, dimension configuration modals, and CSV history export.
- **NTP-based timestamps** on every reading.

## Hardware

| Component | Pins |
|---|---|
| ESP32 | — |
| Load cell + HX711 module | `dout = 4`, `sck = 5` |
| HC-SR04 ultrasonic sensor | `trig = 17`, `echo = 16` |

## Project structure

```
dual-sensor-water-level/
├── codigo_doble_sensor.ino   # ESP32 firmware
└── data/                     # Dashboard served via LittleFS
    ├── index.html
    ├── style.css
    └── app.js
```

## Endpoints

| Endpoint | Method | Description |
|---|---|---|
| `/data` | GET | Load cell reading: weight, level, percentage, tank dimensions, timestamp |
| `/data2` | GET | Ultrasonic reading: distance, level, percentage, reference height, timestamp |
| `/tare` | POST | Remote tare of the load cell |
| `/setDim` | GET | Set tank `radio` and `altura` (load cell) |
| `/setDim2` | GET | Set `alturaRef` (ultrasonic sensor) |

## Setup

1. Install the libraries: `HX711_ADC`, `ArduinoJson`, `LittleFS` (bundled with the ESP32 core).
2. Replace `ssid` and `password` in `codigo_doble_sensor.ino` with your own credentials.
3. Upload the contents of `data/` to LittleFS (LittleFS plugin for Arduino IDE, or `pio run --target uploadfs` in PlatformIO).
4. Flash the `.ino` and open the serial monitor at 57600 baud.
5. Follow the mandatory calibration flow that runs on boot:
   - Tank dimensions (radius and height) for the load cell.
   - Tare + known weight for the load cell.
   - Reference distance + reference height for the ultrasonic sensor.
6. Once connected to WiFi, the local IP is printed to the serial console. Open that IP in a browser to view the dashboard.

## Stack
**Firmware & hardware**
 
<img src="https://img.shields.io/badge/ESP32-000000?style=flat-square&logo=espressif&logoColor=white" alt="ESP32" /> <img src="https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=arduino&logoColor=white" alt="Arduino" /> <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white" alt="C++" /> <img src="https://img.shields.io/badge/HX711-4B4B4B?style=flat-square&logo=arduino&logoColor=white" alt="HX711" />
 
**Dashboard & data**
 
<img src="https://img.shields.io/badge/HTML5-E34F26?style=flat-square&logo=html5&logoColor=white" alt="HTML5" /> <img src="https://img.shields.io/badge/CSS3-1572B6?style=flat-square&logo=css3&logoColor=white" alt="CSS3" /> <img src="https://img.shields.io/badge/JavaScript-F7DF1E?style=flat-square&logo=javascript&logoColor=black" alt="JavaScript" /> <img src="https://img.shields.io/badge/JSON-000000?style=flat-square&logo=json&logoColor=white" alt="JSON" /> <img src="https://img.shields.io/badge/LittleFS-2496ED?style=flat-square&logo=files&logoColor=white" alt="LittleFS" />

## License

MIT — see [LICENSE](./LICENSE).
