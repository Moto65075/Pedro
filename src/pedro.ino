#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#include "esp_sleep.h"
#include "PedroDefinitions.h"
#include "EnergyManager.h"
#include "UIMenu.h"
#include "Menu.h"
#include "Web.h"
#include "ButtonHolder.h"

// ========= TIME AND OTHERS VARIABLES ==========
int ledBrightness = 0;
int ledStep       = 0;
unsigned long lastFadeTime = 0;

bool timeoutStarted = false;
int lastActiveTime = 0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SSD1306> roboEyes(display);
String LOGS;
Menu menu;
PowerOptions powerop = NO_TIMER;
PedroMood pmood;
UIMenu uiMenu(display);
ButtonHolder btnPower(BTN_POWER, 500);   
ButtonHolder btnNext(BTN_NEXT, 800);     

void setupEyes() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  pushLog("INITED EYES");
}

void sleeperTimeout() {
  unsigned long actualTime = millis();

  switch (powerop) {
  case NO_TIMER: 
    if(actualTime - lastActiveTime >= NAPTIME) {
    Serial.println("long time without interact");
    powerop = INTO_SLEEP;
    pmood = SLEEPING;
    roboEyes.close(2, 2);
    roboEyes.setAutoblinker(OFF);
    roboEyes.setIdleMode(OFF);
    roboEyes.setPosition(S);
    lastActiveTime = actualTime;
    timeoutStarted = true;
    }
    break; 
  case INTO_SLEEP:
    if(actualTime - lastActiveTime >= INTOSLEEPTIME) {
    powerop = SLEEP_OVER;
    }
    break;
  case SLEEP_OVER:
    goLightSleep();
    break;
  }
}

void clearTimeout() {
  if(!timeoutStarted) return;
   timeoutStarted = false;
   lastActiveTime = millis();
   roboEyes.open(1, 1);
   roboEyes.setAutoblinker(ON, 3, 2);
   roboEyes.setIdleMode(ON, 2, 2);
   powerop = NO_TIMER;
   pmood = IDLE;
}

void setup() {
  Serial.begin(115200);
  delay(10);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  ledcAttach(STATUS_LED_PIN, LEDC_FREQ, LEDC_RES);
  
  if(cause == ESP_SLEEP_WAKEUP_EXT0) {
    blinkStatusLed(3, 100, 100);
  }

  delay(10);
  
  pushLog("PEDRO READY");
  setupEyes();
  handleWifiManager();
  pmood = IDLE;
  pushLog("POWER ON COMPLETE");
  lastActiveTime = millis();
}

void loop() {
    btnPower.update();
    btnNext.update();

    bool powerShort = btnPower.shortPressed();
    bool powerLong  = btnPower.longPressed();
    bool nextShort  = btnNext.shortPressed();
    
    if(
      powerShort && timeoutStarted ||
      (timeoutStarted && powerop == NAPPING) 
    ) {
      clearTimeout();
      return;
    }
    if(powerShort && powerop == NAPPING) {
      return;
    }
    menu.hold(powerShort, powerLong, nextShort);
    uiMenu.draw(menu);
    if(menu.state() == UIState::IDLE_MODE) {
      sleeperTimeout();
      roboEyes.update();
    }
}