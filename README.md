# ESP32 GPS Vehicle Performance Logger

An Arduino-based project for the ESP32 that measures and displays two key vehicle performance metrics — **0–50 km/h acceleration time** and **braking distance from 50 km/h to standstill** — using a GPS module. Results are served over a local WiFi access point and viewable in any browser.

---

## Hardware Requirements

| Component | Notes |
|---|---|
| ESP32 | Any standard development board |
| GPS Module | Connected to UART2 (RX → pin 16, TX → pin 17) |
| Power Source | USB or vehicle 5V supply |

---

## Software Dependencies

The following libraries are required and must be installed via the Arduino Library Manager or PlatformIO:

- **TinyGPS++** by Mikal Hart — GPS NMEA sentence parsing
- **WiFi / WiFiAP / WiFiClient** — included in the ESP32 Arduino core (no separate installation needed)

---

## How It Works

### Acceleration (0–50 km/h)

The timer starts the moment the GPS-reported speed exceeds **5 km/h** (to filter out GPS noise at standstill). It stops and records the elapsed time in seconds once the speed crosses **50 km/h**. If the speed drops back below 5 km/h before reaching 50 km/h, the attempt is discarded and the timer resets, ready for a clean run.

### Braking Distance (50 → 0 km/h)

When the speed falls below **50 km/h**, the current GPS coordinates are recorded as the braking start point. The end point is recorded when speed falls below **5 km/h**. The straight-line distance between the two points is calculated using the **Haversine formula**, which accounts for Earth's curvature, and the result is stored in metres. If the speed climbs back above 51 km/h before coming to a stop, the measurement is discarded and restarted on the next deceleration event.

### Web Interface

The ESP32 broadcasts a WiFi access point and runs an HTTP server on port **80**. Connect to the network and open a browser to view and refresh your results.

| Setting | Value |
|---|---|
| SSID | `ESP32BOSS` |
| Password | `12345678` |
| Default IP | `192.168.4.1` |

Navigate to `http://192.168.4.1` in your browser. The page will display the last recorded values. Press the **Refresh** button to fetch the latest readings.

---

## Setup & Usage

1. Install the ESP32 board package in Arduino IDE (or configure PlatformIO for ESP32).
2. Install the **TinyGPS++** library.
3. Wire your GPS module to the ESP32:
   - GPS TX → ESP32 pin **16**
   - GPS RX → ESP32 pin **17**
   - GPS VCC → 3.3V or 5V (check your module's datasheet)
   - GPS GND → GND
4. Open `proiect2.ino` and flash it to your ESP32.
5. Connect your phone or laptop to the `ESP32BOSS` WiFi network (password: `12345678`).
6. Open a browser and go to `http://192.168.4.1`.
7. Drive, then press **Refresh** to see your results.

---

## Known Issues

- **Undefined variable `z`** in `printWelcomePage()`: the line `client.print(z)` should read `client.print(HTML_WELCOME)`. As written, the project will not compile without fixing this typo.
- GPS accuracy is subject to satellite fix quality. Ensure the GPS module has a clear sky view before testing.
- The braking distance measurement assumes a roughly straight-line stop. Significant lateral movement during braking will slightly underreport the true distance.

---

## License

This project is provided as-is for educational and personal use. No warranty is expressed or implied.
