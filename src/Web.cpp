#include "Web.h"

WiFiManager wm;
extern Adafruit_SSD1306 display;
extern WiFiClient client;
i2s_chan_handle_t rx_handle;
i2s_chan_handle_t tx_handle;

#define SAMPLE_RATE_T    22050
#define BYTES_PER_SAMPLE 2
#define I2S_FRAME       1024   // bytes escritos no I2S por vez
#define NET_BUFFER      8192   // buffer de rede

void handleWifiManager() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("starting...");
    display.println("waiting wifi....");
    display.println("waiting....");
    display.display();

    delay(50);
    bool res;
    res = wm.autoConnect("Pedro WiFI","2504300");

    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        pushLog("WIFI CONNECTED");
        Serial.println("connected...yeey :)");
    }
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

String makeRequest(String method, const char* host, int port, String uri) {
  WiFiClient client;
  
  // 1. Conexão
  if (!client.connect(host, port)) {
    return "ERRO_CONEXAO";
  }

  // 2. Envia Cabeçalhos
  client.print(method + " " + uri + " HTTP/1.1\r\n");
  client.print("Host: " + String(host) + "\r\n");
  // Importante: Keep-Alive ajuda a não cair a conexão no meio do áudio
  client.print("Connection: keep-alive\r\n\r\n"); 

  // 3. Aguarda resposta do servidor
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      client.stop();
      return "TIMEOUT";
    }
  }

  // 4. Pula os headers da resposta HTTP para chegar no corpo (Body)
  bool headerEnded = false;
  while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
          headerEnded = true;
          break; // Chegamos no corpo da mensagem
      }
  }

  // === AQUI É A DECISÃO ===
  // Se a requisição foi para /encoded, entregamos o controle para o Player
  if (uri.indexOf("match") > -1) {
      processAudio(client); // O código trava aqui até acabar a música
      client.stop();
      return "AUDIO_TOCADO";
  }

  // === SE FOR TEXTO NORMAL (Comandos, JSON) ===
  String payload = "";
  if (headerEnded) {
      // Lê o resto como string normal
      payload = client.readString();
  }
  
  client.stop();
  return payload;
}