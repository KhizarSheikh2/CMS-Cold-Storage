#pragma once
#include <Arduino.h>

// ── Global data ───────────────────────────────────────────────────
float         temperature = 0.0f;
float         humidity    = 0.0f;
unsigned long startTime   = 0;

// ── Screen dimensions (landscape) ────────────────────────────────
#define SCREEN_W 320
#define SCREEN_H 240

// ── Pins ──────────────────────────────────────────────────────────
#define ONE_WIRE_BUS 16    // DS18B20 data pin
#define DHT_PIN      17    // DHT11 data pin

#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define TFT_MOSI 23
#define TFT_SCLK 18

// ── Display ───────────────────────────────────────────────────────
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

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
#define C_CYAN    0x07FF

// ── Data ─────────────────────────────────────────────────────────
extern float temperature;       // DS18B20 reading °C
extern float humidity;          // DHT11 reading %
extern unsigned long startTime; // millis() at boot, for working hours

// ── Timers ────────────────────────────────────────────────────────
unsigned long lastTempRead      = 0;
unsigned long lastDHTRead       = 0;
unsigned long lastDisplayUpdate = 0;