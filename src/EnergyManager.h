#ifndef ENERGYMANAGER_H
#define ENERGYMANAGER_H
#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "driver/i2s_std.h"
#include <Adafruit_SSD1306.h>
#include "Web.h"
#include "PedroDefinitions.h"

extern Adafruit_SSD1306 display;
extern PowerOptions powerop;
extern i2s_chan_handle_t tx_handle;

void enableWakeByButton();
void deepSleep();
void lightSleep();
void pushLog(const String &log);
void fadeOut(int speed);
void drawWaves(int16_t *buffer, size_t length);
void RGB(int r, int g, int b);

#endif