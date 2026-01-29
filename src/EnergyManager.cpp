#include "EnergyManager.h"

extern String LOGS;
extern int lastActiveTime;
extern bool timeoutStarted;
int currentR = 0, currentG = 0, currentB = 0;

void enableWakeByButton() {
  esp_sleep_enable_ext0_wakeup(WAKE_BUTTON_PIN, 1);
}

void deepSleep() {
  RGB(0, 0, 0);
  playSoftTone(329.63, 600, 0.4); 
  Serial.println("go deep sleep no timer...");
  delay(100);

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

  while(currentR != r || currentG != g || currentB != b) {
    
  if(currentR < r) currentR++;
  else if(currentR > r) currentR--;
    
  if(currentG < g) currentG++;
  else if(currentG > g) currentG--;

  if(currentB < b) currentB++;
  else if(currentB > b) currentB--;
  ledcWrite(R_LED_1, 255 - currentR);
  ledcWrite(G_LED_1, 255 - currentG);
  ledcWrite(B_LED_1, 255 - currentB);
  delay(0.5);
  }
}

void sleeperTimeout() {
  unsigned long actualTime = millis();

  switch (powerop) {
  case NO_TIMER: 
    if(actualTime - lastActiveTime >= NAPTIME) {
    Serial.println("long time without interact");
    powerop = INTO_SLEEP;
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
    lightSleep();
    break;
  }
}

void clearTimeout() {
  if(!timeoutStarted) return;
   timeoutStarted = false;
   lastActiveTime = millis();
   powerop = NO_TIMER;
}