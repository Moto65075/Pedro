#pragma once
#include "Adafruit_SSD1306.h"
#include "Fonts/Org_01.h"
#include "EnergyManager.h"
#include "Web.h"
#include "WiFi.h"
#include "Thinker.h"

#define MENU_TIMEOUT_MS 50000

enum class DisplayingType {
    IDLE = 0,
    ON_ACTION,
    ON_MENU
};
void drawMainMenu();
void holdToNextItem();
void executeMenuItem();
void exitMenu();

DisplayingType menuState();