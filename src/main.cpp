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
float temperature  = 0.0f;
float humidity     = 0.0f;
unsigned long startTime = 0;

// ── Landscape Constants (ILI9341) ─────────────────────────────────
#define SCREEN_W 320
#define SCREEN_H 240

// ── Sensor objects ────────────────────────────────────────────────
OneWire           oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DHT               dht(DHT_PIN, DHT11);

// ── Display object ────────────────────────────────────────────────
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// ── Timers ────────────────────────────────────────────────────────
unsigned long lastTempRead     = 0;
unsigned long lastDHTRead      = 0;
unsigned long lastDisplayUpdate = 0;

// ── Forward declarations ──────────────────────────────────────────
void showSplash();
void showMainScreen();
void refreshBoxes();
void drawBox(int x, int y, int w, int h, const char* title, uint16_t accentColor);
void centeredText(const char* txt, uint8_t size, uint16_t color, int refX, int refW, int y);

// ═════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);   // Landscape mode
  tft.fillScreen(C_WHITE);

  ds18b20.begin();
  dht.begin();

  startTime = millis();

  ds18b20.requestTemperatures();
  temperature = ds18b20.getTempCByIndex(0);
  humidity    = dht.readHumidity();
  if (isnan(humidity)) humidity = 0.0f;

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
void showSplash() {
  tft.fillScreen(C_WHITE);
  int x = (SCREEN_W - LOGO_WIDTH) / 2;
  int y = (SCREEN_H - LOGO_HEIGHT) / 2;
  tft.drawRGBBitmap(x, y, logoBitmap, LOGO_WIDTH, LOGO_HEIGHT);
}

void showMainScreen() {
  tft.fillScreen(C_WHITE);

  // Header bar
  tft.fillRect(0, 0, SCREEN_W, 30, C_NAVY);
  tft.fillRect(0, 30, SCREEN_W, 3, C_TEAL);
  centeredText("ROOM MONITOR SYSTEM", 2, C_WHITE, 0, SCREEN_W, 10);

  // Layout for Landscape:
  // Two boxes side-by-side (Top)
  drawBox(10, 55, 145, 100, "TEMPERATURE", C_BLUE);
  drawBox(165, 55, 145, 100, "HUMIDITY", C_BLUE);
  
  // One wide box (Bottom)
  drawBox(10, 165, 300, 65, "WORKING HOURS", C_TEAL);

  refreshBoxes();
}

void refreshBoxes() {
  // Temperature Value
  int tx = 10, ty = 55, tw = 145;
  tft.fillRect(tx + 5, ty + 40, tw - 10, 45, C_WHITE);
  char tVal[8];
  dtostrf(temperature, 4, 1, tVal);
  centeredText(tVal, 3, C_BLUE, tx, tw, ty + 45);
  centeredText("Celsius", 1, C_DKGRAY, tx, tw, ty + 80);

  // Humidity Value
  int hx = 165, hy = 55, hw = 145;
  tft.fillRect(hx + 5, hy + 40, hw - 10, 45, C_WHITE);
  char hVal[8];
  dtostrf(humidity, 4, 1, hVal);
  centeredText(hVal, 3, C_BLUE, hx, hw, hy + 45);
  centeredText("Percent %", 1, C_DKGRAY, hx, hw, hy + 80);

  // Time Value
  int bx = 10, by = 165, bw = 300;
  tft.fillRect(bx + 5, by + 35, bw - 10, 25, C_WHITE);
  unsigned long elapsed = (millis() - startTime) / 1000UL;
  unsigned long hh = elapsed / 3600;
  unsigned long mm = (elapsed % 3600) / 60;
  unsigned long ss = elapsed % 60;
  char timeStr[15];
  snprintf(timeStr, sizeof(timeStr), "%02lu : %02lu : %02lu", hh, mm, ss);
  centeredText(timeStr, 2, C_TEAL, bx, bw, by + 38);
}

void drawBox(int x, int y, int w, int h, const char* title, uint16_t accentColor) {
  // Shadow
  tft.fillRoundRect(x + 2, y + 2, w, h, 6, C_LTGRAY);
  // Body
  tft.fillRoundRect(x, y, w, h, 6, C_WHITE);
  tft.drawRoundRect(x, y, w, h, 6, C_LTGRAY);
  
  // Header stripe inside box
  tft.fillRoundRect(x, y, w, 25, 6, accentColor);
  tft.fillRect(x, y + 15, w, 10, accentColor); // Square bottom of title bar
  
  centeredText(title, 2, C_WHITE, x, w, y + 8);
}

void centeredText(const char* txt, uint8_t size, uint16_t color, int refX, int refW, int y) {
  tft.setTextSize(size);
  tft.setTextColor(color);
  int16_t x1, y1; uint16_t tw, th;
  tft.getTextBounds(txt, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor(refX + (refW - tw) / 2, y);
  tft.print(txt);
}