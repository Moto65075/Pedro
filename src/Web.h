#ifndef WEB_H
#define WEB_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include "EnergyManager.h"
#include "PedroDefinitions.h"

struct Response {
  int code;
  String body;
};

void handleWifiManager();

#endif 