#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#include "variables.h"
#include "logo_full.h"

// ── Global data ───────────────────────────────────────────────────
float         temperature = 0.0f;
float         humidity    = 0.0f;
unsigned long startTime   = 0;

// ── Screen dimensions (landscape) ────────────────────────────────
#define SCREEN_W 320
#define SCREEN_H 240

// ── Sensor objects ────────────────────────────────────────────────
OneWire           oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DHT               dht(DHT_PIN, DHT11);

// ── Display ───────────────────────────────────────────────────────
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// ── Timers ────────────────────────────────────────────────────────
unsigned long lastTempRead      = 0;
unsigned long lastDHTRead       = 0;
unsigned long lastDisplayUpdate = 0;

// ── Forward declarations ──────────────────────────────────────────
void showSplash();
void showMainScreen();
void refreshBoxes();
void drawBox(int x, int y, int w, int h, const char* title, uint16_t accentColor);
void centeredText(const char* txt, uint8_t size, uint16_t color, int refX, int refW, int y);

// ═════════════════════════════════════════════════════════════════
//  Layout constants
//
//  Header:  h=36  (size-2 text fits, ~16px tall + padding)
//  Accent:  h=3
//  Gap:     4px below accent line
//
//  Stripe:  STRIPE_H=34  (size-3 text = 24px tall, +5 top +5 bot)
//  Value:   size-4 text = 32px tall
//
//  Top boxes (x2 side by side):
//    y = 43, h = 86
//    w = 148 each, gap = 16 between, 8 margin each side
//
//  Bottom box (full width):
//    y = 137, h = 90
//    w = 304, x = 8
//
//  Total used: 43 + 86 + 8 + 90 = 227 ≤ 240 ✓
// ═════════════════════════════════════════════════════════════════

#define HEADER_H  36
#define ACCENT_H   3
#define STRIPE_H  34    // tall enough for size-3 title text

#define BOX1_X    8
#define BOX1_Y    (HEADER_H + ACCENT_H + 4)   // 43
#define BOX1_W    148
#define BOX1_H    86

#define BOX2_X    164
#define BOX2_Y    BOX1_Y
#define BOX2_W    148
#define BOX2_H    BOX1_H

#define BOX3_X    8
#define BOX3_Y    (BOX1_Y + BOX1_H + 8)       // 137
#define BOX3_W    304
#define BOX3_H    90

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(C_WHITE);

  ds18b20.begin();
  dht.begin();

  startTime = millis();

  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  if (t != DEVICE_DISCONNECTED_C) temperature = t;

  float h = dht.readHumidity();
  humidity = isnan(h) ? 0.0f : h;

  showSplash();
  delay(2000);
  showMainScreen();
}

void loop() {
  unsigned long now = millis();

  if (now - lastTempRead >= TEMP_INTERVAL) {
    lastTempRead = now;
    ds18b20.requestTemperatures();
    float t = ds18b20.getTempCByIndex(0);
    if (t != DEVICE_DISCONNECTED_C) temperature = t;
  }

  if (now - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = now;
    float h = dht.readHumidity();
    if (!isnan(h)) humidity = h;
  }

  if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    lastDisplayUpdate = now;
    refreshBoxes();
  }
}

// ═════════════════════════════════════════════════════════════════
//  SPLASH
// ═════════════════════════════════════════════════════════════════
void showSplash() {
  tft.fillScreen(C_WHITE);
  int x = (SCREEN_W - LOGO_WIDTH)  / 2;
  int y = (SCREEN_H - LOGO_HEIGHT) / 2;
  tft.drawRGBBitmap(x, y, logoBitmap, LOGO_WIDTH, LOGO_HEIGHT);
}

// ═════════════════════════════════════════════════════════════════
//  MAIN SCREEN
// ═════════════════════════════════════════════════════════════════
void showMainScreen() {
  tft.fillScreen(C_WHITE);

  // ── Header ────────────────────────────────────────────────────
  tft.fillRect(0, 0, SCREEN_W, HEADER_H, C_TEAL);
  tft.fillRect(0, HEADER_H, SCREEN_W, ACCENT_H, C_NAVY);

  // size-2 header text (16px tall), centred vertically in 36px bar
  centeredText("COLD STORAGE MONITOR", 2, C_WHITE, 0, SCREEN_W, (HEADER_H - 16) / 2);   // y = 10

  // ── Card frames ───────────────────────────────────────────────
  drawBox(BOX1_X, BOX1_Y, BOX1_W, BOX1_H, "TEMPERATURE", C_NAVY);
  drawBox(BOX2_X, BOX2_Y, BOX2_W, BOX2_H, "HUMIDITY",    C_NAVY);
  drawBox(BOX3_X, BOX3_Y, BOX3_W, BOX3_H, "RUNNING HOURS", C_RED);

  refreshBoxes();
}

// ═════════════════════════════════════════════════════════════════
//  REFRESH — value areas only
// ═════════════════════════════════════════════════════════════════
void refreshBoxes() {
  int16_t x1, y1;
  uint16_t nw, nh, uw, uh;

  // ── Temperature ───────────────────────────────────────────────
  // Value area: below stripe, 4px padding each side
  tft.fillRect(BOX1_X + 4, BOX1_Y + STRIPE_H + 1,
               BOX1_W - 8, BOX1_H - STRIPE_H - 5, C_WHITE);

  char tVal[8];
  dtostrf(temperature, 4, 1, tVal);

  // Measure size-4 number + size-2 unit
  tft.setTextSize(4);
  tft.getTextBounds(tVal, 0, 0, &x1, &y1, &nw, &nh);   // nh ≈ 32
  tft.setTextSize(2);
  tft.getTextBounds("\xF7""C", 0, 0, &x1, &y1, &uw, &uh);

  int valueAreaH = BOX1_H - STRIPE_H;                   // px available
  int valY       = BOX1_Y + STRIPE_H + (valueAreaH - (int)nh) / 2 - 2;
  int totalW     = (int)nw + 3 + (int)uw;
  int startX     = BOX1_X + (BOX1_W - totalW) / 2;

  tft.setTextSize(4);
  tft.setTextColor(C_NAVY);
  tft.setCursor(startX, valY);
  tft.print(tVal);

  // Unit slightly raised to top-align with number
  tft.setTextSize(3);
  tft.setTextColor(C_NAVY);
  tft.setCursor(startX + (int)nw + 2, valY + 4);
  tft.print("\xF7""C");

  // ── Humidity ──────────────────────────────────────────────────
  tft.fillRect(BOX2_X + 4, BOX2_Y + STRIPE_H + 1, BOX2_W - 8, BOX2_H - STRIPE_H - 5, C_WHITE);

  int humInt = (int)round(humidity);
  char hVal[8];
  snprintf(hVal, sizeof(hVal), "%d", humInt);

  tft.setTextSize(4);
  tft.getTextBounds(hVal, 0, 0, &x1, &y1, &nw, &nh);
  tft.setTextSize(2);
  tft.getTextBounds("%", 0, 0, &x1, &y1, &uw, &uh);

  valY   = BOX2_Y + STRIPE_H + (valueAreaH - (int)nh) / 2 - 2;
  totalW = (int)nw + 3 + (int)uw;
  startX = BOX2_X + (BOX2_W - totalW) / 2;

  tft.setTextSize(4);
  tft.setTextColor(C_NAVY);
  tft.setCursor(startX, valY);
  tft.print(hVal);

  tft.setTextSize(3);
  tft.setTextColor(C_NAVY);
  tft.setCursor(startX + (int)nw + 3, valY + 4);
  tft.print("%");

  // ── Working Hours ─────────────────────────────────────────────
  tft.fillRect(BOX3_X + 4, BOX3_Y + STRIPE_H + 1, BOX3_W - 8, BOX3_H - STRIPE_H - 5, C_WHITE);

  unsigned long elapsed = (millis() - startTime) / 1000UL;
  unsigned long hh = elapsed / 3600;
  unsigned long mm = (elapsed % 3600) / 60;
  unsigned long ss = elapsed % 60;
  char timeStr[16];
  snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", hh, mm, ss);

  tft.setTextSize(4);
  tft.getTextBounds(timeStr, 0, 0, &x1, &y1, &nw, &nh);   // nh ≈ 32

  int box3ValueH = BOX3_H - STRIPE_H;
  int timeY      = BOX3_Y + STRIPE_H + (box3ValueH - (int)nh) / 2 - 2;

  tft.setTextColor(C_RED);
  tft.setCursor(BOX3_X + (BOX3_W - (int)nw) / 2, timeY);
  tft.print(timeStr);
}

// ═════════════════════════════════════════════════════════════════
//  DRAW BOX
// ═════════════════════════════════════════════════════════════════
void drawBox(int x, int y, int w, int h, const char* title, uint16_t accentColor) {
  // Shadow
  tft.fillRoundRect(x + 2, y + 2, w, h, 6, C_LTGRAY);
  // Body
  tft.fillRoundRect(x, y, w, h, 6, C_WHITE);
  tft.drawRoundRect(x, y, w, h, 6, C_LTGRAY);

  // Title stripe — STRIPE_H=34 fits size-3 text (24px) with 5px padding
  tft.fillRoundRect(x, y, w, STRIPE_H, 6, accentColor);
  tft.fillRect(x, y + 16, w, STRIPE_H - 16, accentColor);  // square off bottom

  // Size-3 title, vertically centred in stripe
  // size-3 char height ≈ 24px → top padding = (34-24)/2 = 5
  centeredText(title, 2, C_WHITE, x, w, y + (STRIPE_H - 16) / 2);
}

// ── Utility ───────────────────────────────────────────────────────
void centeredText(const char* txt, uint8_t size, uint16_t color, int refX, int refW, int y) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  int16_t x1, y1; uint16_t tw, th;
  tft.getTextBounds(txt, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor(refX + (refW - tw) / 2, y);
  tft.print(txt);
}