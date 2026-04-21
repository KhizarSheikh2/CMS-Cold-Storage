#pragma once

// ── TFT Display ───────────────────────────────────────────────────
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define TFT_MOSI 23
#define TFT_SCLK 18

// ── Sensors ───────────────────────────────────────────────────────
#define ONE_WIRE_BUS  25   // DS18B20 — safe on WROOM-32U
#define DHT_PIN       26   // DHT11   — safe on WROOM-32U