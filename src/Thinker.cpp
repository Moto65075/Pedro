#include "Thinker.h"

extern i2s_chan_handle_t rx_handle;

static EmotionalMemoryShort memShort;
static EmotionalProfile memProfile;
static EmotionalLongTerm memLong;

static float emotionAffinity[EMOTION_COUNT];

static Emotion currentEmotion = NORMAL;
static Emotion targetEmotion  = NORMAL;
static Environment env        = ENV_CALM;

static uint32_t emotionSince = 0;
static uint32_t emotionHold  = 10000;
static uint32_t lastSoundMs  = 0;

static uint32_t lastAngryTime = 0;

static int32_t micBuffer[MIC_BUFFER];
static int32_t micDC = 0;

uint32_t silenceThreshold;
uint32_t desireEnergy;
uint32_t happyEnergy;
uint32_t angryEnergy;
uint32_t curiousDelta;
uint32_t eerilyVariance;

static uint32_t silenceTime = 0;
static uint32_t lastUpdate = 0;

static Preferences prefs;


static float clamp(float v, float a, float b) {
  if (v < a) return a;
  if (v > b) return b;
  return v;
}

static uint32_t randomHold() {
  return random(EMOTION_MIN_TIME, EMOTION_MAX_TIME);
}

static uint32_t readMicEnergy() {
  size_t bytesRead = 0;

  esp_err_t err = i2s_channel_read(
    rx_handle,
    micBuffer,
    sizeof(micBuffer),
    &bytesRead,
    portMAX_DELAY
  );

  if (err != ESP_OK || bytesRead == 0) return 0;

  int samples = bytesRead / sizeof(int32_t);
  uint32_t energy = 0;

  for (int i = 0; i < samples; i++) {
    // INMP441 → 24 bits válidos
    micBuffer[i] >>= 8;

    // remove DC offset
    micDC = (micDC * 1023 + micBuffer[i]) >> 10;
    micBuffer[i] -= micDC;

    energy += abs(micBuffer[i]);
  }
  static uint32_t smoothedEnergy = 0;
  smoothedEnergy = (smoothedEnergy * 3 + energy) / 4;
  energy = smoothedEnergy;

  return energy / samples; // 🔥 ISSO é o que o cérebro precisa
}

CalibrationData calibrateEnvironment(uint32_t duration_ms) {

  uint32_t start = millis();
  uint32_t samples = 0;

  uint64_t energySum = 0;
  uint64_t deltaSum = 0;

  uint32_t lastEnergy = 0;

  while(millis() - start < duration_ms) {
    uint32_t energy = readMicEnergy();
    energySum += energy;
    if (samples > 0) {
      deltaSum += abs((int32_t)energy - (int32_t)lastEnergy);
    }
    lastEnergy = energy;
    samples++;

    delay(5);
  }

  CalibrationData data;
  data.avgEnergy     = energySum / samples;
  data.variance      = deltaSum / samples;
  data.silenceEnergy = data.avgEnergy * 0.4;

  return data;
}

void applyCalibration(const CalibrationData &c) {
  silenceThreshold = c.silenceEnergy * 0.6;
  desireEnergy     = c.avgEnergy * 0.75;
  happyEnergy      = c.avgEnergy * 1.15;
  angryEnergy      = c.avgEnergy * 1.6;
  curiousDelta     = c.variance * 1.2;
  eerilyVariance   = c.variance * 1.4;
}

static Environment detectEnv(uint32_t e) {
  static uint32_t lastEnergy = 0;
  static uint8_t suddenCount = 0;

  if (e > NOISE_CLAP && (e - lastEnergy) > 5000) {
    suddenCount++;
  } else {
    suddenCount = 0;
  }

  lastEnergy = e;

  if (suddenCount >= 5) {
    suddenCount = 0;
    return ENV_SUDDEN;
  }
  if (e > NOISE_LOUD) return ENV_NOISY;
  if (e > NOISE_HUMAN) return ENV_HUMAN;
  if (e > NOISE_CALM) return ENV_MUSIC;
  return ENV_CALM;
}

static void decayMemory() {
  if(memShort.agitation < 0.6) {
    memShort.agitation *= 0.97;
  } else {
    memShort.agitation *= 0.995;
  }
  memShort.curiosity *= 0.996;
  memShort.happiness *= 0.994;
  memShort.sadness   *= 0.993;
}

static void updateMemory(Environment e) {
  switch (e) {
    case ENV_NOISY:
      memShort.agitation += 0.01;
      memProfile.stress += 0.01;
      memLong.avgNoise  += 0.001;
      break;

    case ENV_MUSIC:
      memShort.happiness += 0.05;
      memProfile.joy += 0.015;
      break;

    case ENV_HUMAN:
      memShort.curiosity += 0.04;
      memProfile.social += 0.01;
      memLong.avgHuman  += 0.001;
      break;

    case ENV_CALM:
      memLong.avgSilence += 0.001;
      if (millis() - lastSoundMs > 30000) {
        memShort.sadness += 0.02;
      }
      break;

    case ENV_SUDDEN:
      memShort.agitation += 0.03;
      break;
  }
}

static Emotion decideEmotion() {

  float angryScore   = memShort.agitation + memProfile.stress + emotionAffinity[MAD];
  float happyScore   = memShort.happiness + memProfile.joy   + emotionAffinity[JOY];
  float curiousScore = memShort.curiosity + memProfile.social + emotionAffinity[THINKING];
  float desireScore  = memShort.sadness   + emotionAffinity[DESIRE];

  static uint32_t noisySince = 0;
  uint32_t now = millis();

  if (env == ENV_NOISY) {
    if (noisySince == 0) noisySince = now;

    if (now - noisySince > 12000) {
      memShort.curiosity += 0.05;
    }
  } else {
    noisySince = 0;
  }

  if (env == ENV_SUDDEN) {
    return SCARED;
  }

  if (
    angryScore > 0.9 &&
    angryScore > happyScore   + 0.2 &&
    angryScore > curiousScore + 0.2 &&
    angryScore > desireScore  + 0.2 &&
    (now - lastAngryTime) > 15000
  ) {
    lastAngryTime = now;
    return MAD;
  }

  if (
    env == ENV_NOISY &&
    silenceTime < 4000 &&         
    curiousScore > 0.4 &&
    angryScore < 0.7 &&
    happyScore < 0.6
  ) {
    return EERILY;
  }

  if (happyScore > 0.7) {
    return JOY;
  }

  if (curiousScore > 0.6) {
    return THINKING;
  }

  if (
    silenceTime > 8000 &&
    desireScore > 0.5 &&
    angryScore < 0.5
  ) {
    return DESIRE;
  }

  return NORMAL;
}


static void applyEmotion(Emotion e) {
  currentEmotion = e;
  emotionSince = millis();
  emotionHold = randomHold();
  emotionAffinity[e] += 0.02;
  float energy = clamp(0.4 + memShort.agitation, 0.3, 0.9);

  switch (e) {
    case JOY:
      RGB(0, 143, 252);
      setEyes(3, false);
      playSoftTone(900, 200, energy);
      break;

    case EERILY:
      RGB(22, 231, 223);
      setEyes(3, false);
      playSoftTone(1300, 300, 0.9);
      break;

    case MAD:
      RGB(255, 60, 0);
      setEyes(2, false);
      playSoftTone(220, 400, 0.5);
      break;

    case THINKING:
      RGB(255, 234, 0);
      setEyes(3, true);
      playSoftTone(720, 150, 0.6);
      break;

    case DESIRE:
      RGB(117, 22, 231);
      setEyes(3, false);
      playSoftTone(300, 600, 0.3);
      break;

    case SCARED:
      emotionHold = random(1200, 3000);
      RGB(255, 255, 255);
      setEyes(4, false);
      setEyes(0, false);
      playSoftTone(1500, 120, 0.95);
      break;

    default:
      RGB(249, 29, 142);
      setEyes(0, false);
      break;
  }
}

static void updateSilence(uint32_t energy) {
  uint32_t now = millis();
  uint32_t dt = now - lastUpdate;
  lastUpdate = now;

  if (energy < silenceThreshold) {
    silenceTime += dt;
  } else {
    silenceTime = 0;
  }
}


void Thinker_Update() {
  uint32_t energy = readMicEnergy();
  env = detectEnv(energy);
  
  if (energy > NOISE_SILENCE) lastSoundMs = millis();

  decayMemory();
  updateMemory(env);

  memShort.agitation = clamp(memShort.agitation, 0.0, 1.0);
  memShort.curiosity = clamp(memShort.curiosity, 0.0, 1.0);
  memShort.happiness = clamp(memShort.happiness, 0.0, 1.0);
  memShort.sadness   = clamp(memShort.sadness, 0.0, 1.0);
  
  updateSilence(energy);

  targetEmotion = decideEmotion();

  if (targetEmotion != currentEmotion &&
      millis() - emotionSince > emotionHold &&
      random(0, 100) > 25) {
    applyEmotion(targetEmotion);
  }

  if (random(0, 1000) > 995) {
    playSoftTone(
      random(200, 1200),
      random(60, 140),
      clamp(0.4 + memShort.curiosity, 0.3, 0.8)
    );
  }
}


void Thinker_Init() {
  randomSeed(esp_random());
  prefs.begin("emo", false);
  
  if(!prefs.isKey("noise")) Thinker_SaveLongTerm();

  memLong.avgNoise   = prefs.getFloat("noise", 0.5);
  memLong.avgSilence = prefs.getFloat("silence", 0.5);
  memLong.avgHuman   = prefs.getFloat("human", 0.5);

  emotionSince = millis();
  lastSoundMs = millis();

  applyEmotion(NORMAL);
}


void Thinker_SaveLongTerm() {
  prefs.putFloat("noise", memLong.avgNoise);
  prefs.putFloat("silence", memLong.avgSilence);
  prefs.putFloat("human", memLong.avgHuman);
}