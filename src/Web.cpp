#include "Web.h"

WiFiManager wm;
extern Adafruit_SSD1306 display;

void handleWifiManager() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("starting...");
    display.println("waiting wifi....");
    display.println("waiting....");
    display.display();

    delay(50);
    bool res;
    res = wm.autoConnect("Pedro WiFI","2504300");

    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        pushLog("WIFI CONNECTED");
        Serial.println("connected...yeey :)");
    }
}