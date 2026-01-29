#ifndef WEB_H
#define WEB_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include "time.h"
#include "mbedtls/base64.h"
#include "driver/i2s_std.h"
#include "EnergyManager.h"
#include "PedroDefinitions.h"

// Definição dos seus pinos específicos
#define I2S_SCK 18
#define I2S_WS  16
#define I2S_SD  19
#define I2S_PORT I2S_NUM_0

#define MIC_BUFFER 64

extern i2s_chan_handle_t rx_handle;

struct Response {
  int code;
  String body;
};

void handleWifiManager();
void startMIC();
void startSpeaker();
String makeRequest(String method, const char* host, int port, String uri);
void processAudio(WiFiClient &client);
void playTone(float freq, int duration_ms);
void playSoftTone(float freq, int duration_ms, float attack);
void streamAudio(bool active);
void testAll();
void playAudio(WiFiClient &client);

#endif 