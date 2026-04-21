#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#include "pins.h"
#include "variables.h"
#include "logo_full.h"

// ── Global data ───────────────────────────────────────────────────
float         temperature = 0.0f;
float         humidity    = 0.0f;
bool          tempError   = false;
bool          humidError  = false;
unsigned long startTime   = 0;

// ── Screen dimensions (landscape) ────────────────────────────────
#define SCREEN_W 320
#define SCREEN_H 240

// ── Layout constants ──────────────────────────────────────────────
#define HEADER_H  36
#define ACCENT_H   3
#define STRIPE_H  34

#define BOX1_X    8
#define BOX1_Y    (HEADER_H + ACCENT_H + 4)
#define BOX1_W    148
#define BOX1_H    86

#define BOX2_X    164
#define BOX2_Y    BOX1_Y
#define BOX2_W    148
#define BOX2_H    BOX1_H

#define BOX3_X    8
#define BOX3_Y    (BOX1_Y + BOX1_H + 8)
#define BOX3_W    304
#define BOX3_H    90

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
//  SETUP
// ═════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(500);  // let serial settle
  Serial.println("=== BOOT ===");

  // ── Display init ──────────────────────────────────────────────
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(C_WHITE);

  // ── Show splash immediately — sensor warmup hides behind it ───
  showSplash();

  // ── DS18B20 init ──────────────────────────────────────────────
  ds18b20.begin();
  ds18b20.setResolution(12);
  ds18b20.setWaitForConversion(true);  // blocking — waits 750ms per read

  // ── DHT11 init — needs 2s minimum before first valid read ─────
  dht.begin();

  // ── Wait for sensors to stabilise (hidden behind splash) ──────
  delay(2500);

  // ── First DS18B20 read ────────────────────────────────────────
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  Serial.print("[DS18B20] Raw: "); Serial.println(t);

  if (t == DEVICE_DISCONNECTED_C || t == -127.0f || t == 85.0f) {
    tempError = true;
    Serial.println("[DS18B20] ERROR — check wiring & 4.7k pull-up on GPIO25");
  } else {
    temperature = t;
    tempError   = false;
    Serial.print("[DS18B20] OK: "); Serial.println(temperature);
  }

  // ── First DHT11 read ──────────────────────────────────────────
  float h = dht.readHumidity();
  Serial.print("[DHT11] Raw: "); Serial.println(h);

  if (isnan(h) || h <= 0.0f || h > 100.0f) {
    humidError = true;
    Serial.println("[DHT11] ERROR — check wiring & GPIO26");
  } else {
    humidity   = h;
    humidError = false;
    Serial.print("[DHT11] OK: "); Serial.println(humidity);
  }

  startTime = millis();
  showMainScreen();
}

// ═════════════════════════════════════════════════════════════════
//  LOOP
// ═════════════════════════════════════════════════════════════════
void loop() {
  unsigned long now = millis();

  // ── DS18B20 read every 5s ─────────────────────────────────────
  if (now - lastTempRead >= TEMP_INTERVAL) {
    lastTempRead = now;
    ds18b20.requestTemperatures();          // blocking — 750ms wait built in
    float t = ds18b20.getTempCByIndex(0);

    if (t == DEVICE_DISCONNECTED_C || t == -127.0f || t == 85.0f) {
      tempError = true;
      Serial.println("[DS18B20] ERROR");
    } else {
      temperature = t;
      tempError   = false;
      Serial.print("[DS18B20] "); Serial.println(temperature);
    }
  }

  // ── DHT11 read every 5s ───────────────────────────────────────
  if (now - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = now;
    float h = dht.readHumidity();

    if (isnan(h) || h <= 0.0f || h > 100.0f) {
      humidError = true;
      Serial.println("[DHT11] ERROR");
    } else {
      humidity   = h;
      humidError = false;
      Serial.print("[DHT11] "); Serial.println(humidity);
    }
  }

  // ── Refresh display every 1s ──────────────────────────────────
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

  tft.fillRect(0, 0, SCREEN_W, HEADER_H, C_TEAL);
  tft.fillRect(0, HEADER_H, SCREEN_W, ACCENT_H, C_NAVY);
  centeredText("COLD STORAGE MONITOR", 2, C_WHITE, 0, SCREEN_W, (HEADER_H - 16) / 2);

  drawBox(BOX1_X, BOX1_Y, BOX1_W, BOX1_H, "TEMPERATURE",  C_NAVY);
  drawBox(BOX2_X, BOX2_Y, BOX2_W, BOX2_H, "HUMIDITY",     C_NAVY);
  drawBox(BOX3_X, BOX3_Y, BOX3_W, BOX3_H, "RUNNING HOURS", C_RED);

  refreshBoxes();
}

// ═════════════════════════════════════════════════════════════════
//  REFRESH — redraws value areas only
// ═════════════════════════════════════════════════════════════════
void refreshBoxes() {
  int16_t  x1, y1;
  uint16_t nw, nh, uw, uh;
  int      valueAreaH = BOX1_H - STRIPE_H;

  // ── Temperature ───────────────────────────────────────────────
  tft.fillRect(BOX1_X + 4, BOX1_Y + STRIPE_H + 1,
               BOX1_W - 8, BOX1_H - STRIPE_H - 5, C_WHITE);

  if (tempError) {
    centeredText("888", 4, C_RED, BOX1_X, BOX1_W,
                 BOX1_Y + STRIPE_H + (valueAreaH - 32) / 2 - 2);
  } else {
    char tVal[8];
    dtostrf(temperature, 4, 1, tVal);

    tft.setTextSize(4);
    tft.getTextBounds(tVal, 0, 0, &x1, &y1, &nw, &nh);
    tft.setTextSize(3);
    tft.getTextBounds("\xF7""C", 0, 0, &x1, &y1, &uw, &uh);

    int totalW = (int)nw + 3 + (int)uw;
    int startX = BOX1_X + (BOX1_W - totalW) / 2;
    int valY   = BOX1_Y + STRIPE_H + (valueAreaH - (int)nh) / 2 - 2;

    tft.setTextSize(4);
    tft.setTextColor(C_NAVY);
    tft.setCursor(startX, valY);
    tft.print(tVal);

    tft.setTextSize(3);
    tft.setTextColor(C_TEAL);
    tft.setCursor(startX + (int)nw + 3, valY + 4);
    tft.print("\xF7""C");
  }

  // ── Humidity ──────────────────────────────────────────────────
  tft.fillRect(BOX2_X + 4, BOX2_Y + STRIPE_H + 1,
               BOX2_W - 8, BOX2_H - STRIPE_H - 5, C_WHITE);

  if (humidError) {
    centeredText("888", 4, C_RED, BOX2_X, BOX2_W,
                 BOX2_Y + STRIPE_H + (valueAreaH - 32) / 2 - 2);
  } else {
    int  humInt = (int)round(humidity);
    char hVal[8];
    snprintf(hVal, sizeof(hVal), "%d", humInt);

    tft.setTextSize(4);
    tft.getTextBounds(hVal, 0, 0, &x1, &y1, &nw, &nh);
    tft.setTextSize(3);
    tft.getTextBounds("%", 0, 0, &x1, &y1, &uw, &uh);

    int totalW = (int)nw + 3 + (int)uw;
    int startX = BOX2_X + (BOX2_W - totalW) / 2;
    int valY   = BOX2_Y + STRIPE_H + (valueAreaH - (int)nh) / 2 - 2;

    tft.setTextSize(4);
    tft.setTextColor(C_NAVY);
    tft.setCursor(startX, valY);
    tft.print(hVal);

    tft.setTextSize(3);
    tft.setTextColor(C_TEAL);
    tft.setCursor(startX + (int)nw + 3, valY + 4);
    tft.print("%");
  }

  // ── Running Hours ─────────────────────────────────────────────
  tft.fillRect(BOX3_X + 4, BOX3_Y + STRIPE_H + 1,
               BOX3_W - 8, BOX3_H - STRIPE_H - 5, C_WHITE);

  unsigned long elapsed = (millis() - startTime) / 1000UL;
  unsigned long hh = elapsed / 3600;
  unsigned long mm = (elapsed % 3600) / 60;
  unsigned long ss = elapsed % 60;
  char timeStr[16];
  snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", hh, mm, ss);

  int box3ValueH = BOX3_H - STRIPE_H;
  tft.setTextSize(4);
  tft.getTextBounds(timeStr, 0, 0, &x1, &y1, &nw, &nh);
  int timeY = BOX3_Y + STRIPE_H + (box3ValueH - (int)nh) / 2 - 2;

  tft.setTextColor(C_RED);
  tft.setCursor(BOX3_X + (BOX3_W - (int)nw) / 2, timeY);
  tft.print(timeStr);
}

// ═════════════════════════════════════════════════════════════════
//  DRAW BOX
// ═════════════════════════════════════════════════════════════════
void drawBox(int x, int y, int w, int h,
             const char* title, uint16_t accentColor) {
  tft.fillRoundRect(x + 2, y + 2, w, h, 6, C_LTGRAY);
  tft.fillRoundRect(x, y, w, h, 6, C_WHITE);
  tft.drawRoundRect(x, y, w, h, 6, C_LTGRAY);

  tft.fillRoundRect(x, y, w, STRIPE_H, 6, accentColor);
  tft.fillRect(x, y + 16, w, STRIPE_H - 16, accentColor);

  centeredText(title, 2, C_WHITE, x, w, y + (STRIPE_H - 16) / 2);
}

// ── Utility ───────────────────────────────────────────────────────
void centeredText(const char* txt, uint8_t size, uint16_t color,
                  int refX, int refW, int y) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  int16_t x1, y1; uint16_t tw, th;
  tft.getTextBounds(txt, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor(refX + (refW - tw) / 2, y);
  tft.print(txt);
}