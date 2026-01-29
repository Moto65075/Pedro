#include "Web.h"

WiFiManager wm;

extern Adafruit_SSD1306 display;
extern WiFiClient client;

extern bool is_connected;

const char* server_ip = "192.168.1.5";
const uint16_t server_port = 80;

const int CHUNK_SIZE = 8192; 
uint8_t audio_buffer[CHUNK_SIZE];
int buffer_idx = 0;

i2s_chan_handle_t rx_handle;
i2s_chan_handle_t tx_handle;

void handleWifiManager() {
    
    RGB(146, 31, 228);
    bool res;
    res = wm.autoConnect("Pedro WiFI","2504300");
    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        pushLog("WIFI CONNECTED");
        Serial.println("connected...yeey :)");
        const char* ntpServer = "pool.ntp.org";
        const long  gmtOffset_sec = -3;
        const int   daylightOffset_sec = 3600;

        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        Serial.println("started hours");
    }
}

String makeRequest(String method, const char* host, int port, String uri) {
  WiFiClient client;
  
  if (!client.connect(host, port)) {
    return "ERRO_CONEXAO";
  }
  
  client.print(method + " " + uri + " HTTP/1.1\r\n");
  client.print("Host: " + String(host) + "\r\n");
  client.print("Connection: keep-alive\r\n\r\n"); 

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      client.stop();
      return "TIMEOUT";
    }
  }

  bool headerEnded = false;
  while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
          headerEnded = true;
          break; 
      }
  }

  if (uri.indexOf("match") > -1) {
      playAudio(client);
      client.stop();
      return "AUDIO_TOCADO";
  }
  
  String payload = "";
  if (headerEnded) {
      payload = client.readString();
  }
  
  client.stop();
  return payload;
}

void startMIC() {
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000), 
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    
            .bclk = (gpio_num_t)I2S_BCK_PIN,
            .ws   = (gpio_num_t)I2S_WS_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din  = (gpio_num_t)I2S_DATA_PIN,
        },
    };

    // Força o ESP32 a ler o slot da esquerda (pino L/R no GND)
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    i2s_channel_init_std_mode(rx_handle, &std_cfg); // Use RX especificamente
    i2s_channel_enable(rx_handle);
}

void startSpeaker() {
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_1,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 8,
        .dma_frame_num = 256,
        .auto_clear = true,
    };

    i2s_new_channel(&chan_cfg, &tx_handle, NULL);

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
           .sample_rate_hz = 22050,
           .clk_src = I2S_CLK_SRC_APLL,
           .mclk_multiple = I2S_MCLK_MULTIPLE_256
        },

        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT,
            I2S_SLOT_MODE_MONO
        ),

        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)I2S_SPEAKER_BCLK,
            .ws   = (gpio_num_t)I2S_SPEAKER_LRC,
            .dout = (gpio_num_t)I2S_SPEAKER_DIN,
            .din  = I2S_GPIO_UNUSED
        }
    };

    i2s_channel_init_std_mode(tx_handle, &std_cfg);
    i2s_channel_enable(tx_handle);
}

void playTone(float freq, int duration_ms) {
    size_t bytes_written;
    int samples = (44100 * duration_ms) / 1000;
    int16_t sample;

    for(int i = 0; i < samples; i++) {
        sample = (int16_t)(10000 * sin(2 * PI * freq * i / 44100));
        i2s_channel_write(tx_handle, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
}

void playSoftTone(float freq, int duration_ms, float attack = 0.2) {
  size_t bytes_written;
  int samples = (44100 * duration_ms) / 1000;
  int16_t sample;

  for (int i = 0; i < samples; i++) {
    float progress = (float)i / samples;
    float amplitude_factor;

    if (progress < attack) {
      amplitude_factor = progress / attack; 
    } else {
      amplitude_factor = 1.0 - ((progress - attack) / (1.0 - attack)); // Desce o volume
    }

    sample = (int16_t)(8000 * amplitude_factor * sin(2 * PI * freq * i / 44100));
    i2s_channel_write(tx_handle, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
  }
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

void testAll() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Testing...");
    display.display();

  RGB(255, 0, 0);
  delay(1000);
  RGB(0, 255, 0);
  delay(1000);
  RGB(0, 0, 255);
  delay(1000);
  RGB(255, 255, 255);
  delay(1000);

  for (float f = 200; f < 2000; f += 100) {
    playSoftTone(f, 50, 0.1);
  }

  delay(200);

  for (int i = 0; i < 3; i++) {
    playSoftTone(1000, 100, 0.05);
  }

  delay(1000);
}

void playAudio(WiFiClient &client) {
    static uint8_t buffer[2048];
    size_t bytes_written;

    while (client.connected() || client.available()) {
        size_t len = client.read(buffer, sizeof(buffer));
        
        if(len <= 0) continue;

        len &= ~1;

        if(len == 0) continue;

        i2s_channel_write(tx_handle, buffer, len, &bytes_written, portMAX_DELAY);
    }
}