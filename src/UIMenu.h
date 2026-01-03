#ifndef UI_MENU_H
#define UI_MENU_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Menu.h"
#include "EnergyManager.h"
#include "PedroDefinitions.h"
#include "Web.h"

class UIMenu {
public:
    UIMenu(Adafruit_SSD1306 &display);

    // desenha conforme estado do Menu
    void draw(const Menu &menu);

private:
    Adafruit_SSD1306 &_display;

    void drawIdle(const Menu &menu);
    void drawMainMenu(const Menu &menu);
    void drawPowerMenu(const Menu &menu);
    void drawMainSection(const Menu &menu);   // Info / Tests / Logs
    void drawPowerSection(const Menu &menu);  // modos de energia
};

#endif