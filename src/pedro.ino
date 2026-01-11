#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#include <WiFi.h>
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

const char* server_ip = "192.168.1.8";

const uint16_t server_port = 80;
bool is_connected = false;

const int CHUNK_SIZE = 8192; 
uint8_t audio_buffer[CHUNK_SIZE];
int buffer_idx = 0;

bool timeoutStarted = false;
int lastActiveTime = 0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SSD1306> roboEyes(display);
String LOGS;
Menu menu;
PowerOptions powerop = NO_TIMER;
PedroMood pmood;
WiFiClient client;
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
    lightSleep();
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

void streamAudio(bool active) {
    if (!active) {
        if (is_connected) {
            // No modo Chunked, o fim da transmissão é indicado por um chunk de tamanho zero
            client.print("0\r\n\r\n"); 
            client.stop();
            is_connected = false;
            Serial.println("Streaming finalizado e enviado para /tts.");
        }
        return;
    }

    if (!is_connected) {
        if (client.connect(server_ip, server_port)) {
            is_connected = true;

            // --- CABEÇALHO HTTP MANUAL ---
            client.print("POST /tts HTTP/1.1\r\n"); // Aqui definimos a ROTA
            client.print("Host: "); client.print(server_ip); client.print("\r\n");
            client.print("Content-Type: application/octet-stream\r\n");
            client.print("Transfer-Encoding: chunked\r\n"); // Permite enviar áudio sem saber o tamanho final
            client.print("Connection: keep-alive\r\n");
            client.print("\r\n"); // Fim do cabeçalho
            
            Serial.println("Conectado à rota /tts!");
        } else {
            return; 
        }
    }

    int32_t raw_samples[128];
    size_t bytes_read = 0;

    if (i2s_channel_read(rx_handle, raw_samples, sizeof(raw_samples), &bytes_read, 10) == ESP_OK) {
        int samples = bytes_read / sizeof(int32_t);

        for (int i = 0; i < samples; i++) {
            int16_t sample16 = (int16_t)(raw_samples[i] >> 14);

            audio_buffer[buffer_idx++] = (uint8_t)(sample16 & 0xFF);
            audio_buffer[buffer_idx++] = (uint8_t)((sample16 >> 8) & 0xFF);

            if (buffer_idx >= CHUNK_SIZE) {
                if (client.connected()) {
                    // --- FORMATO HTTP CHUNKED ---
                    // 1. Envia o tamanho do chunk em Hexadecimal
                    client.print(String(CHUNK_SIZE, HEX));
                    client.print("\r\n");
                    // 2. Envia os dados binários do áudio
                    client.write(audio_buffer, CHUNK_SIZE);
                    client.print("\r\n");
                } else {
                    is_connected = false;
                }
                buffer_idx = 0;
            }
        }
    }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  ledcAttach(STATUS_LED_PIN, LEDC_FREQ, LEDC_RES);
  
  if(cause == ESP_SLEEP_WAKEUP_EXT0) {
     fadeOut(5);
  }

  delay(10);
  
  pushLog("PEDRO READY");
  setupEyes();
  handleWifiManager();
  startMIC();
  startSpeaker();
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
    bool nextLong   = btnNext.longPressed();

    if(nextLong) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Listening...");
      display.display();
      //streamAudio(true);
      
      makeRequest("GET", server_ip, server_port, "/match/manchild");
     
      return;
    }
    /*
    if(is_connected && !nextLong) {
      pmood = IDLE;
      streamAudio(false);
    }*/

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