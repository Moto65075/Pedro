#ifndef THINKER_H
#define THINKER_H

#include <Preferences.h>
#include <driver/i2s_std.h>
#include "Thinker.h"
#include "EnergyManager.h"

#define MIC_BUFFER_SAMPLES 128
#define MIC_SAMPLE_RATE 16000

#define NOISE_SILENCE 800
#define NOISE_CALM    2000
#define NOISE_HUMAN   4000
#define NOISE_LOUD    7000
#define NOISE_CLAP    12000

#define EMOTION_MIN_TIME 8000
#define EMOTION_MAX_TIME 20000


enum Emotion {
  JOY,
  MAD,
  THINKING,
  NORMAL,
  DESIRE,
  EERILY,
  SCARED,
  EMOTION_COUNT
};

enum Environment {
  ENV_CALM,
  ENV_HUMAN,
  ENV_MUSIC,
  ENV_NOISY,
  ENV_SUDDEN
};

struct CalibrationData {
  uint32_t silenceEnergy;
  uint32_t avgEnergy;
  uint32_t variance;
};

struct EmotionalMemoryShort {
  float agitation;
  float curiosity;
  float happiness;
  float sadness;
};

struct EmotionalProfile {
  float stress;
  float social;
  float joy;
};

struct EmotionalLongTerm {
  float avgNoise;
  float avgSilence;
  float avgHuman;
};


void setEyes(int eyeType, bool thinking);
void Thinker_Update();
void Thinker_Init();
void Thinker_SaveLongTerm();

CalibrationData calibrateEnvironment(uint32_t duration_ms);
void applyCalibration(const CalibrationData &c);

#endif