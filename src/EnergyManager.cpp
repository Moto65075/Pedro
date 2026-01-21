#include "EnergyManager.h"

extern String LOGS;

void enableWakeByButton() {
  esp_sleep_enable_ext0_wakeup(WAKE_BUTTON_PIN, 1);
}

void deepSleep() {
  RGB(0, 0, 0);
  playSoftTone(329.63, 600, 0.4); 
  Serial.println("go deep sleep no timer...");
  delay(100);

  fadeOut(5);

  enableWakeByButton();
  display.clearDisplay();
  display.display();

  esp_deep_sleep_start();
}

void lightSleep() {
  RGB(0, 0, 0);
  playSoftTone(523.25, 300, 0.2); 
  playSoftTone(349.23, 500, 0.5); 
  Serial.println("into light sleep...");
  
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  
  powerop = NAPPING;
  enableWakeByButton();
  if(tx_handle) {
    i2s_channel_disable(tx_handle);
    i2s_del_channel(tx_handle);
    tx_handle = NULL;
  }
  esp_wifi_stop();
  esp_light_sleep_start();
  
  startSpeaker();
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

void RGB(int r, int g, int b) {
  ledcWrite(R_LED_1, 255 - r);
  ledcWrite(G_LED_1, 255 - g);
  ledcWrite(B_LED_1, 255 - b);
}

void fadeOut(int speed) {
  for (int brightness = 80; brightness >= 0; brightness -= speed) {
    if (brightness < 0) brightness = 0;
    delay(FADE_INTERVAL);
  }
}