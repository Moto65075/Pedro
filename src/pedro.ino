#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#include <WiFi.h>
#include "time.h"
#include "Thinker.h"
#include "esp_sleep.h"
#include "PedroDefinitions.h"
#include "EnergyManager.h"
#include "Menu.h"
#include "Web.h"
#include "ButtonHolder.h"

// ========= TIME AND OTHERS VARIABLES ==========
int ledBrightness = 0;
int ledStep       = 0;
unsigned long lastFadeTime = 0;
bool is_connected = false;
int lastActiveTime = 0;
bool timeoutStarted = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SSD1306> roboEyes(display);
String LOGS;
PowerOptions powerop = NO_TIMER;
WiFiClient client;
ButtonHolder btnCenter(BTN_POWER, 500);   
ButtonHolder btnNext(BTN_NEXT, 800);    

void setupPeriferics() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.close(2, 2);
  roboEyes.setAutoblinker(OFF);
  roboEyes.setIdleMode(OFF);
  roboEyes.setPosition(S);

  ledcAttach(R_LED_1, LEDC_FREQ, LEDC_RES);
  ledcAttach(G_LED_1, LEDC_FREQ, LEDC_RES);
  ledcAttach(B_LED_1, LEDC_FREQ, LEDC_RES);

  roboEyes.update();
  pushLog("started peripherics");

  handleWifiManager();
  startMIC();
  startSpeaker();
  randomSeed(analogRead(0));
  playSoftTone(329.00, 300, 0.1);  
  playSoftTone(523.25, 500, 0.2);

  CalibrationData calib = calibrateEnvironment(500);
  applyCalibration(calib);

  Thinker_Init();

  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.update();

  lastActiveTime = millis();
}

void setEyes(int eyeType, bool thinking) {
  
  if(eyeType == 4) roboEyes.anim_confused();
  else if(thinking) roboEyes.setCuriosity(ON);
  else {
    roboEyes.setMood(eyeType);
    roboEyes.setCuriosity(OFF);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  
  if(cause == ESP_SLEEP_WAKEUP_EXT0) {
    RGB(0, 0, 0);
  }

  delay(10);
  
  pushLog("PEDRO starting...");
  setupPeriferics();
}

void loop() {
  btnCenter.update();
  btnNext.update();

  bool btnCenterLong = btnCenter.longPressed();
  bool btnNextLong   = btnNext.longPressed();
  bool btnCenterShort = btnCenter.shortPressed();
  bool btnNextShort   = btnNext.shortPressed();

  if(btnNextLong && !is_connected) {
    RGB(255, 0, 255);
    makeRequest("GET", "192.168.1.5", 80, "/match/to");
  }

  if(btnCenterLong && menuState() == DisplayingType::IDLE && powerop != INTO_SLEEP) drawMainMenu();
  if(btnNextShort && menuState() == DisplayingType::ON_MENU) holdToNextItem();
  if(
    btnCenterShort && 
   (
    menuState() == DisplayingType::ON_MENU ||
    menuState() == DisplayingType::ON_ACTION
   )
  ) executeMenuItem();
  else if(
    btnCenter.shortPressed() && 
    powerop == INTO_SLEEP
  ) {
    roboEyes.setMood(HAPPY);
    clearTimeout();
  }

  unsigned long actualTime = millis();

  if(
    menuState() == DisplayingType::ON_MENU &&
    actualTime - lastActiveTime >= MENU_TIMEOUT_MS
  ) {
    lastActiveTime = actualTime;
    exitMenu();
  }

  if(menuState() == DisplayingType::IDLE && powerop == NO_TIMER) {
  roboEyes.update();
  Thinker_Update();
  sleeperTimeout();
  }

  if(powerop == INTO_SLEEP) {
    display.clearDisplay();
    display.display();
    sleeperTimeout();
  }
}