#include "Menu.h"

extern Adafruit_SSD1306 display;
extern String LOGS;
extern int lastActiveTime;

int itemIndex = 0;
DisplayingType display_type = DisplayingType::IDLE;

void drawMainMenu() {
    display_type = DisplayingType::ON_MENU;
    delay(50);
    RGB(30, 50, 200);
display.clearDisplay();
display.setTextColor(1);
display.setTextWrap(false);
display.setFont(&Org_01);
display.setCursor(4, 9);
display.print("MENU");

if(itemIndex == 0) display.fillCircle(6, 22, 2, 1);
else display.drawCircle(6, 22, 2, 1);

display.setCursor(12, 22);
display.print("Infos");

if(itemIndex == 1) display.fillCircle(6, 34, 2, 1);
else display.drawCircle(6, 34, 2, 1);

display.setCursor(12, 34);
display.print("Outputs");

if(itemIndex == 2) display.fillCircle(6, 46, 2, 1);
else display.drawCircle(6, 46, 2, 1);

display.setCursor(12, 46);
display.print("Tests");

if(itemIndex == 3) display.fillCircle(6, 57, 2, 1);
else display.drawCircle(6, 57, 2, 1);

display.setCursor(12, 57);
display.print("RGB");

if(itemIndex == 4) display.fillCircle(64, 22, 2, 1);
display.drawCircle(64, 22, 2, 1);

display.setCursor(70, 22);
display.print("Power off");

if(itemIndex == 5) display.fillCircle(64, 34, 2, 1);
display.drawCircle(64, 34, 2, 1);

display.setCursor(70, 34);
display.print("Naptime");

if(itemIndex == 6) display.fillCircle(64, 46, 2, 1);
display.drawCircle(64, 46, 2, 1);

display.setCursor(70, 46);
display.print("Exit");

display.display();
}

void holdToNextItem() {
  itemIndex++;
  if(itemIndex > 6) itemIndex = 0;
  drawMainMenu();
  lastActiveTime = millis();
}

void executeMenuItem() {
  if(display_type == DisplayingType::ON_ACTION) {
    display_type = DisplayingType::ON_MENU;
    drawMainMenu();
    return;
  }
  display_type = DisplayingType::ON_ACTION;
  switch (itemIndex) {
    case 0: {
      display.clearDisplay();
      display.setTextColor(1);
      display.setTextWrap(false);
      display.setFont(&Org_01);
      display.setCursor(4, 8);
      display.print("Infos");
      display.setFont();
      display.setCursor(4, 24);
      display.print("version: Coubs 1.2");
      display.setCursor(4, 36);
      display.print(WiFi.SSID());
      display.setCursor(4, 51);
      display.print("center - return");
      display.display();
      
      break;
    }
    case 1: {
      display.clearDisplay();
      display.setTextColor(1);
      display.setTextWrap(false);
      display.setFont(&Org_01);
      display.setCursor(4, 8);
      display.print("Outputs");
      display.setFont();
      display.setCursor(4, 24);
      display.print(LOGS);
      display.setCursor(4, 51);
      display.print("center - return");
      display.display();

      break;
    }
    case 2: {
      testAll();
      display_type = DisplayingType::ON_MENU;
      drawMainMenu();
      break;
    }
    case 3: {

      display.clearDisplay();
      display.setTextColor(1);
      display.setTextWrap(false);
      display.setFont(&Org_01);
      display.setCursor(4, 8);
      display.print("Calibrating....");
      display.display();

      CalibrationData calib = calibrateEnvironment(10000);
      applyCalibration(calib);

      display.clearDisplay();
      display.setTextColor(1);
      display.setTextWrap(false);
      display.setFont(&Org_01);
      display.setCursor(4, 8);
      display.print("Done!");
      display.setFont();
      display.setCursor(4, 24);
      display.print("Avg Energy: " + String(calib.avgEnergy));
      display.setCursor(4, 36);
      display.print("Variance : " + String(calib.variance));
      display.setCursor(4, 51);
      display.print("center - return");
      display.display();

      break;
    }
    case 4: {
      deepSleep();
      break;
    }
    case 5: {
      lightSleep();
      exitMenu();
      break;
    }
    case 6: 
      exitMenu();
      break;
  }
}

void exitMenu() {
    display_type = DisplayingType::IDLE;
    display.clearDisplay();
    display.display();
    delay(50);
    RGB(0, 0, 0);
}

DisplayingType menuState() {
    return display_type;
}