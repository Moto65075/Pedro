#ifndef ENERGYMANAGER_H
#define ENERGYMANAGER_H
#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Adafruit_SSD1306.h>
#include "PedroDefinitions.h"

extern Adafruit_SSD1306 display;
extern PowerOptions powerop;

void enableWakeByButton();
void deepSleep();
void lightSleep();
void pushLog(const String &log);
void fadeOut(int speed);

#endif