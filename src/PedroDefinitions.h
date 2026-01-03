#ifndef PEDRODEFINITIONS_H
#define PEDRODEFINITIONS_H

#define OLED_SDA GPIO_NUM_21
#define OLED_SCL GPIO_NUM_22
#define WAKE_BUTTON_PIN GPIO_NUM_0
#define BTN_POWER  GPIO_NUM_0
#define BTN_NEXT   GPIO_NUM_5
#define MIC_PIN    GPIO_NUM_34
#define I2S_BCK_PIN GPIO_NUM_26
#define I2S_WS_PIN GPIO_NUM_27
#define I2S_DATA_PIN GPIO_NUM_25

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define LONG_PRESS_TIME 500 

#define STATUS_LED_PIN 4
#define NAPTIME 120000
#define INTOSLEEPTIME 50000

const int LEDC_FREQ = 5000;
const int LEDC_RES = 8;

extern int ledBrightness;
extern int ledStep;
extern unsigned long lastFadeTime;
const unsigned long FADE_INTERVAL = 40;

enum PedroMood {
  IDLE,
  CURIOUS,
  SLEEPING,
  PLAYING,
  PAUSED,
  STOPPED,
  SKIPPED,
  NO_MATCH,
  ERROR
};

enum PowerOptions {
  AUDIO_WAITING,
  UPDATE_WAITING,
  MENU_WAITING,
  NO_TIMER,
  INTO_SLEEP,
  NAPPING,
  SLEEP_OVER
};

extern PedroMood pmood;
extern String LOGS;

#endif