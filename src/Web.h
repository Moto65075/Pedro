#ifndef WEB_H
#define WEB_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include "driver/i2s_std.h"
#include "EnergyManager.h"
#include "PedroDefinitions.h"

// Definição dos seus pinos específicos
#define I2S_SCK 18
#define I2S_WS  16
#define I2S_SD  19
#define I2S_PORT I2S_NUM_0

extern i2s_chan_handle_t rx_handle;

// Configuração da amostragem
#define SAMPLE_RATE 16000 

struct Response {
  int code;
  String body;
};

void handleWifiManager();
void setupI2s();
int32_t readMIC();

#endif 