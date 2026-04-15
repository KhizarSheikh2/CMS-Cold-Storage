# CMS-Cold-Storage

A Cold Storage Monitoring System developed using **PlatformIO** in **VSCode**. This project is designed to run on an **ESP32** microcontroller and provides a real-time visual dashboard to monitor the environmental conditions of a cold storage facility.

## Features
- **Real-Time Temperature Monitoring:** Uses a DS18B20 temperature sensor for accurate readings.
- **Humidity Tracking:** Integrates a DHT sensor (e.g., DHT11) to monitor ambient moisture levels.
- **Running Hours:** Keeps track of the system uptime displayed as an intuitive `HH:MM:SS` timer.
- **Graphical Dashboard:** A beautifully designed UI displayed on an ILI9341 TFT display, featuring dedicated cards for various sensor metrics.
- **Custom Splash Screen:** Displays a custom logo during the boot-up sequence.

## Hardware Requirements
- **Microcontroller:** ESP32 (Target profile: `esp32doit-devkit-v1`)
- **Display:** TFT LCD with ILI9341 driver IC
- **Temperature Sensor:** DS18B20
- **Humidity Sensor:** DHT11 or DHT22

## Software Dependencies
The project uses the following libraries (managed automatically via PlatformIO):
- `Adafruit ILI9341` - For the TFT display
- `Adafruit GFX Library` - Core graphics routines
- `DallasTemperature` - For handling the DS18B20 sensor
- `OneWire` - Communication protocol for DS18B20
- `DHT sensor library` - For the DHT humidity/temperature sensor

## Getting Started

1. **Clone the Repository** and open the folder in **VSCode**.
2. **Install PlatformIO:** Ensure you have the PlatformIO IDE extension installed in VSCode.
3. **Wiring Connections:** Ensure your ESP32 is properly connected to the TFT display, DHT sensor, and DS18B20 according to the pin definitions in `src/variables.h`.
4. **Build and Upload:**
   - Click the **PlatformIO: Build** checkmark `✓` at the bottom taskbar.
   - Connect your ESP32 via USB and click the **PlatformIO: Upload** arrow `→`.
5. **Monitor:** Need debugging or serial output? Click the **PlatformIO: Serial Monitor** plug icon.

## Project Structure
- `src/` - Contains the main source code files (`main.cpp`, `variables.h`, `logo_full.h`).
- `include/` - Project header files.
- `lib/` - Custom or private libraries.
- `platformio.ini` - PlatformIO configuration file managing environments and library dependencies.

## License
Feel free to use and adapt this code for your own monitoring solutions!
