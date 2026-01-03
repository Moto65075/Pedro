#include "UIMenu.h"

extern WiFiManager wm;

UIMenu::UIMenu(Adafruit_SSD1306 &display)
: _display(display) {}

void UIMenu::draw(const Menu &menu) {
    switch (menu.state()) {
        case UIState::IDLE_MODE:
            drawIdle(menu);
            break;
        case UIState::MAIN_MENU:
            if (menu.sectionOpened()) drawMainSection(menu);
            else drawMainMenu(menu);
            break;
        case UIState::POWER_MENU:
            if (menu.sectionOpened()) drawPowerSection(menu);
            else drawPowerMenu(menu);
            break;
    }
}

void UIMenu::drawIdle(const Menu &) {
  ledcWrite(STATUS_LED_PIN, 50);
}

void UIMenu::drawMainMenu(const Menu &menu) {
    static const char *opts[] = { "Info", "WiFi", "LOGS", "Exit" };

    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    _display.println("MAIN MENU");
    _display.println("");

    int idx = menu.currentIndex();
    for (int i = 0; i < 4; ++i) {
        if (i == idx) {
            _display.fillRect(0, 12 + i * 10, 128, 9, WHITE);
            _display.setTextColor(SSD1306_BLACK);
        } else {
            _display.setTextColor(SSD1306_WHITE);
        }
        _display.setCursor(5, 13 + i * 10);
        _display.println(opts[i]);
    }

    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 54);
    _display.print("Power: SEL  Next: MOVE");
    _display.display();
}

void UIMenu::drawPowerMenu(const Menu &menu) {
    static const char *opts[] = { "Idle", "Naptime", "Night", "Power Off" };

    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    _display.println("POWER MENU");
    _display.println("");

    int idx = menu.currentIndex();
    for (int i = 0; i < 4; ++i) {
        if (i == idx) {
            _display.fillRect(0, 12 + i * 10, 128, 9, SSD1306_WHITE);
            _display.setTextColor(SSD1306_BLACK);
        } else {
            _display.setTextColor(SSD1306_WHITE);
        }
        _display.setCursor(5, 13 + i * 10);
        _display.println(opts[i]);
    }

    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 54);
    _display.print("Power: SEL  Next: MOVE");
    _display.display();
}

void UIMenu::drawMainSection(const Menu &menu) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);

    switch (menu.currentIndex()) {
        case 0: // Info
            _display.println("FIRMWARE & Tests");
            _display.println("Coubs 1.3");
            _display.println("OLED: " + String(SCREEN_WIDTH) + "x" + String(SCREEN_HEIGHT));
            _display.println("WiFiManager v34");
            _display.println("Battery: N/A");
            break;
        case 1: // Tests
            _display.println("WiFi");
            if(WiFi.isConnected()) {
                _display.println("Connected");
                _display.println(WiFi.SSID());
                _display.println(WiFi.localIP().toString());
            } else {
                _display.println("Not connected");
            }
            break;
        case 2: // Logs
            _display.println("LOGS");
            _display.println("" + LOGS);
            break;
    }

    _display.println("");
    _display.println("Power - return");
    _display.display();
}

void UIMenu::drawPowerSection(const Menu &menu) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);

    switch (menu.currentIndex()) {
        case 0:
            _display.println("IDLE MODE");
            _display.println("Olhos + Awake");
            break;
        case 1:
            _display.println("NIGHT SLEEP");
            _display.println("Brilho baixo / sleep");
            break;
    }

    _display.println("");
    _display.println("Power - return");
    _display.display();
}