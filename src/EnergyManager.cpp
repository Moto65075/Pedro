#include "EnergyManager.h"

extern String LOGS;

void enableWakeByButton() {
  esp_sleep_enable_ext0_wakeup(WAKE_BUTTON_PIN, 1);
}

void deepSleep() {
  Serial.println("go deep sleep no timer...");
  delay(100);

  fadeOut(5);

  enableWakeByButton();
  digitalWrite(STATUS_LED_PIN, LOW);
  display.clearDisplay();
  display.display();

  esp_deep_sleep_start();
}

void lightSleep() {
  fadeOut(5);
  Serial.println("into light sleep...");
  
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  
  powerop = NAPPING;
  enableWakeByButton();
  esp_wifi_stop();
  esp_light_sleep_start();
  esp_wifi_start();

  display.ssd1306_command(SSD1306_DISPLAYON);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  powerop = NO_TIMER;
}


/*  

_ Utils

*/
void pushLog(const String &log) {
  LOGS += log + " \n";
}

void fadeOut(int speed) {
  for (int brightness = 80; brightness >= 0; brightness -= speed) {
    if (brightness < 0) brightness = 0;
    ledcWrite(STATUS_LED_PIN, brightness);
    delay(FADE_INTERVAL);
  }
}