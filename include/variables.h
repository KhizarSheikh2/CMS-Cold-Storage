#pragma once
#include <Arduino.h>

// ── Pins ──────────────────────────────────────────────────────────
#define ONE_WIRE_BUS 16    // DS18B20 data pin
#define DHT_PIN      17    // DHT11 data pin

#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define TFT_MOSI 23
#define TFT_SCLK 18

// ── Display ───────────────────────────────────────────────────────
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ── Sensor read intervals ─────────────────────────────────────────
#define TEMP_INTERVAL    5000   // ms between DS18B20 reads
#define DHT_INTERVAL     5000   // ms between DHT11 reads
#define DISPLAY_INTERVAL 1000   // ms between display refresh

// ── Colours (RGB565) ──────────────────────────────────────────────
#define C_WHITE   0xFFFF
#define C_BLACK   0x0000
#define C_NAVY    0x0010
#define C_LTGRAY  0xC618
#define C_DKGRAY  0x7BEF
#define C_GREEN   0x07E0
#define C_RED     0xF800
#define C_BLUE    0x001F
#define C_ORANGE  0xFD20
#define C_TEAL    0x0410

// ── Data ─────────────────────────────────────────────────────────
extern float temperature;       // DS18B20 reading °C
extern float humidity;          // DHT11 reading %
extern unsigned long startTime; // millis() at boot, for working hours