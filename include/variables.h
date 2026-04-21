#pragma once
#include <Arduino.h>

// ── Display (landscape) ───────────────────────────────────────────
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// ── Sensor intervals ──────────────────────────────────────────────
#define TEMP_INTERVAL    5000
#define DHT_INTERVAL     5000
#define DISPLAY_INTERVAL 1000

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

// ── Sensor data ───────────────────────────────────────────────────
extern float         temperature;
extern float         humidity;
extern bool          tempError;
extern bool          humidError;
extern unsigned long startTime;